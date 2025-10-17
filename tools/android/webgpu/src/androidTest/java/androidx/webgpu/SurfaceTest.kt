package androidx.webgpu

import android.graphics.SurfaceTexture
import android.view.Surface
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.SmallTest
import androidx.webgpu.helper.ValidationException
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
@SmallTest
class SurfaceTest {

  companion object {
    private const val WIDTH = 600
    private const val HEIGHT = 800
  }

  private lateinit var surfaceTexture: SurfaceTexture
  private lateinit var validSurface: Surface
  private lateinit var webGpu: WebGpu

  @Before
  fun setup() {
    surfaceTexture = SurfaceTexture(false)
    surfaceTexture.setDefaultBufferSize(WIDTH, HEIGHT)
    validSurface = Surface(surfaceTexture)

    webGpu = runBlocking { createWebGpu(validSurface) }
  }

  @After
  fun teardown() {
    validSurface.release()
    surfaceTexture.release()
    webGpu.close()
  }

  /**
   * Tests the basic surface lifecycle.
   * This test verifies that a surface can be successfully:
   * 1. Created from an Android Surface.
   * 2. Configured with valid parameters (dimensions, format).
   * 3. Used to acquire a texture that matches the configuration.
   * 4. Presented successfully to the underlying system.
   */
  @Test
  fun surfaceLifecycle_configureAndPresent_succeeds() {
    val surface = webGpu.webgpuSurface
    val config = SurfaceConfiguration(
      device = webGpu.device,
      format = TextureFormat.RGBA8Unorm,
      height = HEIGHT,
      width = WIDTH
    )
    surface.configure(config)
    // ensure no errors are thrown
    val currentTexture = surface.getCurrentTexture().texture
    assertEquals(currentTexture.format, TextureFormat.RGBA8Unorm)
    assertEquals(currentTexture.height, HEIGHT)
    assertEquals(currentTexture.width, WIDTH)

    val status = surface.present()
    assertEquals("Present should be successful", Status.Success, status)
  }

  /**
   * Verifies that configuring a surface with invalid parameters (e.g., negative width)
   * results in a validation error.
   */
  @Test
  fun configure_withInvalidParameters_fails() {
    val surface = webGpu.webgpuSurface
    val invalidConfig = SurfaceConfiguration(
      device = webGpu.device,
      format = TextureFormat.RGBA8Unorm,
      height = HEIGHT,
      width = -1 // Invalid parameter
    )

    // Assert that calling configure with this invalid descriptor throws an error.
    assertThrows(ValidationException::class.java) {
      surface.configure(invalidConfig)
    }
  }
}