package androidx.webgpu

import androidx.test.filters.SmallTest
import androidx.webgpu.helper.ValidationException
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import java.nio.ByteBuffer
import junit.framework.TestCase
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test

@Suppress("UNUSED_VARIABLE")
@SmallTest
class CommandEncoderTest {

  private lateinit var device: GPUDevice
  private lateinit var webGpu: WebGpu

  @Before
  fun setup() = runBlocking {
    webGpu = createWebGpu()
    device = webGpu.device
  }

  @After
  fun teardown() {
    runCatching { device.destroy() }
    webGpu.close()

  }

  /**
   * Verifies that a command encoder cannot be used after `finish()` has been called.
   */
  @Test
  fun testFinish() {
    val encoder = device.createCommandEncoder()
    val unusedCommandBuffer = encoder.finish()

    val dummyBuffer = device.createBuffer(
      BufferDescriptor(size = 4, usage = BufferUsage.CopySrc)
    )
    assertThrows(
      "Using a finished encoder should throw an error",
      ValidationException::class.java
    ) {
      encoder.copyBufferToBuffer(dummyBuffer, 0, dummyBuffer, 0, 4)
    }
    dummyBuffer.destroy()
  }

  /**
   * Ensures that a balanced `pushDebugGroup`/`popDebugGroup` pair is a valid operation.
   *
   * This test uses an error scope to assert that no validation errors occur when pushing
   * and immediately popping a debug group on a command encoder.
   */
  @Test
  fun testDebugGroups() {
    val encoder = device.createCommandEncoder()

    device.pushErrorScope(ErrorFilter.Validation)
    encoder.pushDebugGroup("MyDebugGroup")
    encoder.popDebugGroup()
    val error = runBlocking { device.popErrorScope() }

    TestCase.assertTrue(
      "Expected no error for balanced push/pop debug group",
      error.status == PopErrorScopeStatus.Success &&
        error.type == ErrorType.NoError
    )
    val unusedCommandBuffer = encoder.finish()
  }

  /**
   * Verifies that `finish()` fails if a nested pass was attempted.
   *
   * This confirms that attempting to begin a second pass before the first has ended
   * invalidates the encoder, causing a validation error upon calling `finish()`.
   */
  @Test
  fun finish_failsWhenNestedPassWasAttempted() {
    val encoder = device.createCommandEncoder()
    val activePassEncoder = encoder.beginComputePass()

    val unusedComputePassEncoder = encoder.beginComputePass()

    activePassEncoder.end()

    device.pushErrorScope(ErrorFilter.Validation)
    val unusedCommandBuffer = encoder.finish()
    val error = runBlocking { device.popErrorScope() }

    TestCase.assertTrue(
      "Expected a validation error on .finish() due to an earlier nested pass attempt",
      error.type == ErrorType.Validation,
    )
  }

  /**
   * Verifies that `beginRenderPass` successfully creates a `GPURenderPassEncoder`.
   *
   * This test provides a valid `RenderPassDescriptor` and asserts that the
   * returned encoder is not null, confirming the command's successful initiation.
   */
  @Test
  fun testBeginRenderPass() {
    val texture = device.createTexture(
      TextureDescriptor(
        size = Extent3D(1, 1, 1),
        format = TextureFormat.RGBA8Unorm,
        usage = TextureUsage.RenderAttachment
      )
    )
    val textureView = texture.createView()

    val encoder = device.createCommandEncoder()
    val passEncoder = encoder.beginRenderPass(
      RenderPassDescriptor(
        colorAttachments = arrayOf(
          RenderPassColorAttachment(
            view = textureView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = Color(0.0, 0.0, 0.0, 1.0)
          )
        )
      )
    )
    TestCase.assertNotNull(passEncoder)
    passEncoder.end()
    val unusedCommandBuffer = encoder.finish()
  }

  /**
   * Verifies that a render pass with `LoadOp.Clear` correctly clears a texture to a specific color.
   */
  @Test
  fun testBeginRenderPass_clearsTextureCorrectly() {
    val queue = device.getQueue()

    val textureWidth = 1
    val textureHeight = 1
    val textureFormat = TextureFormat.RGBA8Unorm

    val bytesPerPixel = 4
    val bufferSize = (textureWidth * textureHeight * bytesPerPixel).toLong()

    val renderTexture = device.createTexture(
      TextureDescriptor(
        size = Extent3D(textureWidth, textureHeight, 1),
        format = textureFormat,
        usage = TextureUsage.RenderAttachment or TextureUsage.CopySrc
      )
    )
    val textureView = renderTexture.createView()

    val readbackBuffer = device.createBuffer(
      BufferDescriptor(
        size = bufferSize,
        usage = BufferUsage.CopyDst or BufferUsage.MapRead
      )
    )

    val clearColor = Color(0.2, 0.8, 0.6, 1.0)

    val encoder = device.createCommandEncoder()

    // This is the operation we are testing.
    val passEncoder = encoder.beginRenderPass(
      RenderPassDescriptor(
        colorAttachments = arrayOf(
          RenderPassColorAttachment(
            view = textureView,
            loadOp = LoadOp.Clear,
            storeOp = StoreOp.Store,
            clearValue = clearColor
          )
        )
      )
    )
    passEncoder.end()

    encoder.copyTextureToBuffer(
      source = TexelCopyTextureInfo(texture = renderTexture),
      destination = TexelCopyBufferInfo(
        buffer = readbackBuffer,
        layout = TexelCopyBufferLayout()
      ),
      copySize = Extent3D(textureWidth, textureHeight, 1)
    )

    val commandBuffer = encoder.finish()

    queue.submit(arrayOf(commandBuffer))

    val mapResult = runBlocking { readbackBuffer.mapAsync(MapMode.Read, 0, bufferSize) }
    TestCase.assertEquals(MapAsyncStatus.Success, mapResult.status)

    val mappedData: ByteBuffer = readbackBuffer.getConstMappedRange(0, bufferSize)

    val expectedBytes = byteArrayOf(
      (clearColor.r * 255).toInt().toByte(),
      (clearColor.g * 255).toInt().toByte(),
      (clearColor.b * 255).toInt().toByte(),
      (clearColor.a * 255).toInt().toByte()
    )

    val actualBytes = ByteArray(bytesPerPixel)
    mappedData.get(actualBytes)

    assertArrayEquals(
      "The bytes read back from the texture do not match the expected clear color.",
      expectedBytes,
      actualBytes
    )

    readbackBuffer.unmap()
    readbackBuffer.destroy()
    renderTexture.destroy()
  }

  /**
   * Verifies that the `clearBuffer` command can be successfully encoded without validation errors.
   *
   * This test pushes a validation error scope, encodes the `clearBuffer` command for a valid buffer
   * and range, and then pops the error scope to assert that the operation was valid.
   */
  @Test
  fun testClearBuffer() {
    val buffer = device.createBuffer(
      BufferDescriptor(
        size = 16,
        usage = BufferUsage.CopyDst
      )
    )

    val encoder = device.createCommandEncoder()
    device.pushErrorScope(ErrorFilter.Validation)
    encoder.clearBuffer(buffer, 0, 16)
    val error = runBlocking { device.popErrorScope() }

    TestCase.assertTrue(
      "Expected clearBuffer to succeed",
      error.status == PopErrorScopeStatus.Success &&
        error.type == ErrorType.NoError
    )
    val unusedCommandBuffer = encoder.finish()
  }

  /**
   * Validates that the `resolveQuerySet` command can be encoded without errors.
   *
   * This test creates a `QuerySet` and a destination buffer, then encodes a command to
   * resolve the query results into the buffer. It uses an error scope to assert that
   * the command is considered a valid operation by the encoder.
   */
  @Test
  fun testResolveQuerySet() {
    val querySet = device.createQuerySet(
      QuerySetDescriptor(
        type = QueryType.Occlusion,
        count = 1
      )
    )

    val destination = device.createBuffer(
      BufferDescriptor(
        size = 8, // Occlusion queries are 64-bit (8 bytes)
        usage = BufferUsage.QueryResolve
      )
    )

    val encoder = device.createCommandEncoder()
    device.pushErrorScope(ErrorFilter.Validation)
    encoder.resolveQuerySet(querySet, 0, 1, destination, 0)
    val error = runBlocking { device.popErrorScope() }

    TestCase.assertTrue(
      "Expected resolveQuerySet to succeed",
      error.status == PopErrorScopeStatus.Success &&
        error.type == ErrorType.NoError
    )
    val unusedCommandBuffer = encoder.finish()
  }

  /**
   * Ensures that `insertDebugMarker` can be encoded without validation errors.
   *
   * This test verifies that adding a debug marker, a non-functional command used for debugging
   * and profiling, is a valid operation within a command encoder. An error scope is used to
   * confirm the success of the command encoding.
   */
  @Test
  fun testInsertDebugMarker() {
    val encoder = device.createCommandEncoder()

    device.pushErrorScope(ErrorFilter.Validation)
    encoder.insertDebugMarker("MyDebugMarker")
    val error = runBlocking { device.popErrorScope() }

    TestCase.assertTrue(
      "Expected insertDebugMarker to succeed",
      error.status == PopErrorScopeStatus.Success &&
        error.type == ErrorType.NoError
    )
    val unusedCommandBuffer = encoder.finish()
  }
}
