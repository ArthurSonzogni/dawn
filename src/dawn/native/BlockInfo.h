// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_BLOCK_INFO_H_
#define SRC_DAWN_NATIVE_BLOCK_INFO_H_

#include "dawn/common/TypedInteger.h"
#include "dawn/native/Format.h"

namespace dawn::native {

// Strong types for texel and block counts.
// These are used to avoid computation errors that occur when using regular integers
// for texel and block values, and mixing them up in computations, particularly for block
// sizes greater than 1x1.
// By using strong types, we cannot mix these two types in computations, and must use
// conversion functions provided in TypedTexelBlockInfo.

// TexelCount and BlockCount are uint64_t, not uint32_t, because as typed
// integers, they do not participate in type promotion to uint64_t, which
// is being relied on for computing buffer offsets and such.
using TexelCount = dawn::TypedInteger<struct TexelCountTag, uint64_t>;
using BlockCount = dawn::TypedInteger<struct BlockCountTag, uint64_t>;

// Strong type version of Origin3D, which is always in texel space
struct TexelOrigin3D {
    TexelCount x{0};
    TexelCount y{0};
    TexelCount z{0};

    // Construct from input values
    // NOLINTNEXTLINE: allow implicit constructor
    TexelOrigin3D(TexelCount x = TexelCount{0},
                  TexelCount y = TexelCount{0},
                  TexelCount z = TexelCount{0})
        : x(x), y(y), z(z) {}

    // Implicitly convert from Origin3D as Origin3D is always in texel space
    // NOLINTNEXTLINE: allow implicit constructor
    TexelOrigin3D(const Origin3D& o) : x(o.x), y(o.y), z(o.z) {}

    // Convert to Origin3D
    Origin3D ToOrigin3D() const {
        return {static_cast<uint32_t>(x), static_cast<uint32_t>(y), static_cast<uint32_t>(z)};
    }
};

// Stores an origin in block space
struct BlockOrigin3D {
    BlockCount x{0};
    BlockCount y{0};
    BlockCount z{0};

    // Construct from input values
    // NOLINTNEXTLINE: allow implicit constructor
    BlockOrigin3D(BlockCount x = BlockCount{0},
                  BlockCount y = BlockCount{0},
                  BlockCount z = BlockCount{0})
        : x(x), y(y), z(z) {}
};

// Strong type version of Extent3D.
struct TexelExtent3D {
    TexelCount width;
    TexelCount height{1};
    TexelCount depthOrArrayLayers{1};

    // Default constructor
    TexelExtent3D() = default;

    // Construct from input values
    // NOLINTNEXTLINE: allow implicit constructor
    TexelExtent3D(TexelCount width,
                  TexelCount height = TexelCount{1},
                  TexelCount depthOrArrayLayers = TexelCount{1})
        : width(width), height(height), depthOrArrayLayers(depthOrArrayLayers) {}

    // Implicitly convert from Extent3D as Extent3D is always in texel space
    // NOLINTNEXTLINE: allow implicit constructor
    TexelExtent3D(const Extent3D& e)
        : width(e.width), height(e.height), depthOrArrayLayers(e.depthOrArrayLayers) {}

    // Convert to Extent3D
    Extent3D ToExtent3D() const {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                static_cast<uint32_t>(depthOrArrayLayers)};
    }
};

// Stores an extent in block space
struct BlockExtent3D {
    BlockCount width;
    BlockCount height{1};
    BlockCount depthOrArrayLayers{1};

    // Default constructor
    BlockExtent3D() = default;

    // Construct from input values
    // NOLINTNEXTLINE: allow implicit constructor
    BlockExtent3D(BlockCount width,
                  BlockCount height = BlockCount{1},
                  BlockCount depthOrArrayLayers = BlockCount{1})
        : width(width), height(height), depthOrArrayLayers(depthOrArrayLayers) {}
};

// Strong type version of TexelBlockInfo that stores the dimensions of the block
// as TexelCounts, and provides conversion functions between texels, blocks, and bytes.
struct TypedTexelBlockInfo {
    uint32_t byteSize;
    TexelCount width;
    TexelCount height;

    // Default constructor
    TypedTexelBlockInfo() = default;

    // Convert from TexelBlockInfo
    // NOLINTNEXTLINE: allow implicit constructor
    TypedTexelBlockInfo(const TexelBlockInfo& blockInfo)
        : byteSize(blockInfo.byteSize), width(blockInfo.width), height(blockInfo.height) {}

    // Convert to TexelBlockInfo
    TexelBlockInfo ToTexelBlockInfo() const {
        return {byteSize, static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    // Convert blocks to bytes
    uint64_t ToBytes(BlockCount value) const { return static_cast<uint64_t>(value) * byteSize; }

    // Convert bytes to blocks
    BlockCount BytesToBlocks(uint64_t bytes) const {
        DAWN_ASSERT(bytes % byteSize == 0);
        return BlockCount{bytes / byteSize};
    }

    // Convert texel height to block height
    BlockCount ToBlockHeight(TexelCount value) const {
        DAWN_ASSERT(value % height == TexelCount{0});
        return BlockCount{static_cast<uint64_t>(value / height)};
    }

    // Convert from texel width to block width
    BlockCount ToBlockWidth(TexelCount value) const {
        DAWN_ASSERT(value % width == TexelCount{0});
        return BlockCount{static_cast<uint64_t>(value / width)};
    }

    // Convert from texel depth to block depth
    BlockCount ToBlockDepth(TexelCount value) const {
        // TODO(amaiorano): When we add block 'depth' for 3D block support, divide this value by
        // 'depth'
        return BlockCount{static_cast<uint64_t>(value)};
    }

    // Convert from block width to texel width
    TexelCount ToTexelWidth(BlockCount value) const {
        return TexelCount{static_cast<uint64_t>(value)} * width;
    }

    // Convert from block height to texel height
    TexelCount ToTexelHeight(BlockCount value) const {
        return TexelCount{static_cast<uint64_t>(value)} * height;
    }

    // Convert from block depth to texel depth
    TexelCount ToTexelDepth(BlockCount value) const {
        // TODO(amaiorano): When we add block 'depth' for 3D block support, multiply this value by
        // 'depth'
        return TexelCount{static_cast<uint64_t>(value)};
    }

    // Convert from TexelOrigin3D to BlockOrigin3D
    BlockOrigin3D ToBlock(const TexelOrigin3D& origin) {
        return {ToBlockWidth(origin.x), ToBlockHeight(origin.y), ToBlockDepth(origin.z)};
    }

    // Convert from TexelExtent3D to BlockExtent3D
    BlockExtent3D ToBlock(const TexelExtent3D& extent) {
        return {ToBlockWidth(extent.width), ToBlockHeight(extent.height),
                ToBlockDepth(extent.depthOrArrayLayers)};
    }

    // Convert from BlockOrigin3D to TexelOrigin3D
    TexelOrigin3D ToTexel(const BlockOrigin3D& origin) {
        return {ToTexelWidth(origin.x), ToTexelHeight(origin.y), ToTexelDepth(origin.z)};
    }

    // Convert from BlockExtent3D to TexelExtent3D
    TexelExtent3D ToTexel(const BlockExtent3D& extent) {
        return {ToTexelWidth(extent.width), ToTexelHeight(extent.height),
                ToTexelDepth(extent.depthOrArrayLayers)};
    }
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLOCK_INFO_H_
