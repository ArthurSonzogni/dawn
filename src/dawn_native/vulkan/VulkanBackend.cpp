// Copyright 2019 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// VulkanBackend.cpp: contains the definition of symbols exported by VulkanBackend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

// Include vulkan_platform.h before VulkanBackend.h includes vulkan.h so that we use our version
// of the non-dispatchable handles.
#include "common/vulkan_platform.h"

#include "dawn_native/VulkanBackend.h"

#include "common/SwapChainUtils.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/NativeSwapChainImplVk.h"
#include "dawn_native/vulkan/TextureVk.h"

namespace dawn_native { namespace vulkan {

    VkInstance GetInstance(WGPUDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return backendDevice->GetVkInstance();
    }

    VkDevice GetDevice(WGPUDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return backendDevice->GetVkDevice();
    }

    VkQueue GetQueue(WGPUDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return backendDevice->GetQueue();
    }

    DAWN_NATIVE_EXPORT PFN_vkVoidFunction GetInstanceProcAddr(WGPUDevice device,
                                                              const char* pName) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return (*backendDevice->fn.GetInstanceProcAddr)(backendDevice->GetVkInstance(), pName);
    }

    // Explicitly export this function because it uses the "native" type for surfaces while the
    // header as seen in this file uses the wrapped type.
    DAWN_NATIVE_EXPORT DawnSwapChainImplementation
    CreateNativeSwapChainImpl(WGPUDevice device, VkSurfaceKHRNative surfaceNative) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        VkSurfaceKHR surface = VkSurfaceKHR::CreateFromHandle(surfaceNative);

        DawnSwapChainImplementation impl;
        impl = CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, surface));
        impl.textureUsage = WGPUTextureUsage_Present;

        return impl;
    }

    WGPUTextureFormat GetNativeSwapChainPreferredFormat(
        const DawnSwapChainImplementation* swapChain) {
        NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
        return static_cast<WGPUTextureFormat>(impl->GetPreferredFormat());
    }

#ifdef DAWN_PLATFORM_LINUX
    WGPUTexture WrapVulkanImageOpaqueFD(WGPUDevice cDevice,
                                        WGPUTexture cTexture,
                                        const ExternalImageDescriptorOpaqueFD* descriptor) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        Texture* texture = reinterpret_cast<Texture*>(cTexture);

        TextureBase* textureOut = device->CreateTextureWrappingVulkanImage(
            texture, descriptor, descriptor->memoryFD, descriptor->waitFDs);

        return reinterpret_cast<WGPUTexture>(textureOut);
    }

    void ImportImageWaitFDs(WGPUDevice cDevice, WGPUTexture cTexture, std::vector<int> waitFDs) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        Texture* texture = reinterpret_cast<Texture*>(cTexture);

        MaybeError result = device->ImportImageWaitFDs(texture, waitFDs);
        (void)result;
    }

    int ExportSignalSemaphoreOpaqueFD(WGPUDevice cDevice, WGPUTexture cTexture, bool destroy) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        Texture* texture = reinterpret_cast<Texture*>(cTexture);

        if (!texture) {
            return -1;
        }

        ExternalSemaphoreHandle outHandle;
        if (device->ConsumedError(device->SignalAndExportExternalTexture(texture, &outHandle, destroy))) {
            return -1;
        }

        return outHandle;
    }

    WGPUTexture WrapVulkanImageVkImage(WGPUDevice cDevice,
                                       const ExternalImageDescriptorVkImage* descriptor) {
        Device* device = reinterpret_cast<Device*>(cDevice);
        TextureBase* textureOut = device->CreateTextureWrappingVulkanImage(descriptor, descriptor->image, descriptor->waitSemaphores);
        return reinterpret_cast<WGPUTexture>(textureOut);
    }
#endif

}}  // namespace dawn_native::vulkan
