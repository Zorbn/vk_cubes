#include "buffer.hpp"

Buffer Buffer::create(VmaAllocator allocator, size_t size, VkDeviceSize byteSize, VkBufferUsageFlags usage, bool cpuAccessable) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = byteSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (cpuAccessable) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    Buffer buffer;
    buffer.byteSize = byteSize;
    buffer.size = size;

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.allocInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    return buffer;
}

void Buffer::copyTo(VmaAllocator& allocator, VkQueue graphicsQueue, VkDevice device, Commands& commands, Buffer& dst) {
    VkCommandBuffer commandBuffer = commands.beginSingleTimeCommands(graphicsQueue, device);

    VkBufferCopy copyRegion{};
    copyRegion.size = dst.byteSize;
    vkCmdCopyBuffer(commandBuffer, buffer, dst.buffer, 1, &copyRegion);

    commands.endSingleTimeCommands(commandBuffer, graphicsQueue, device);
}

void Buffer::copyToImage(Commands& commands, VkQueue graphicsQueue, VkDevice device, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = commands.beginSingleTimeCommands(graphicsQueue, device);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    commands.endSingleTimeCommands(commandBuffer, graphicsQueue, device);
}

Buffer Buffer::fromVertices(VmaAllocator allocator, Commands& commands, VkQueue graphicsQueue, VkDevice device, const std::vector<Vertex>& vertices) {
    VkDeviceSize bufferByteSize = sizeof(vertices[0]) * vertices.size();

    Buffer stagingBuffer = create(allocator, vertices.size(), bufferByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
    stagingBuffer.setData(vertices.data());

    Buffer vertexBuffer = create(allocator, vertices.size(), bufferByteSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false);

    stagingBuffer.copyTo(allocator, graphicsQueue, device, commands, vertexBuffer);
    stagingBuffer.destroy(allocator);

    return vertexBuffer;
}

Buffer Buffer::fromIndices(VmaAllocator allocator, Commands& commands, VkQueue graphicsQueue, VkDevice device, const std::vector<uint16_t>& indices) {
    VkDeviceSize bufferByteSize = sizeof(indices[0]) * indices.size();

    Buffer stagingBuffer = create(allocator, indices.size(), bufferByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
    stagingBuffer.setData(indices.data());

    Buffer indexBuffer = create(allocator, indices.size(), bufferByteSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, false);

    stagingBuffer.copyTo(allocator, graphicsQueue, device, commands, indexBuffer);
    stagingBuffer.destroy(allocator);

    return indexBuffer;
}

void Buffer::destroy(VmaAllocator& allocator) {
    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::setData(const void* data) {
    memcpy(allocInfo.pMappedData, data, byteSize);
}