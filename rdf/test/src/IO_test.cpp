/* Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. */
#include <catch.hpp>

#include "amdrdf.h"

#include <cstring>
#include "test_rdf.h"

namespace
{
struct MemoryStream
{
    std::int64_t currentOffset = 0;
    std::vector<unsigned char> buffer;
};

int MemoryStreamWrite(void* p, std::int64_t count, const void* buffer, std::int64_t* bytesWritten)
{
    MemoryStream* ms = static_cast<MemoryStream*>(p);
    ms->buffer.resize(ms->currentOffset + count);
    ::memcpy(ms->buffer.data() + ms->currentOffset, buffer, count);
    ms->currentOffset += count;

    if (bytesWritten) {
        *bytesWritten = count;
    }

    return rdfResultOk;
}

int MemoryStreamRead(void* p, std::int64_t count, void* buffer, std::int64_t* bytesRead)
{
    MemoryStream* ms = static_cast<MemoryStream*>(p);
    auto bytesToRead = std::min(count, static_cast<std::int64_t>(ms->buffer.size()) - ms->currentOffset);
    ::memcpy(buffer, ms->buffer.data() + ms->currentOffset, bytesToRead);
    ms->currentOffset += bytesToRead;

    if (bytesRead) {
        *bytesRead = bytesToRead;
    }

    return rdfResultOk;
}

int MemoryStreamTell(void* p, std::int64_t* position)
{
    MemoryStream* ms = static_cast<MemoryStream*>(p);
    *position = ms->currentOffset;
    return rdfResultOk;
}

int MemoryStreamSeek(void* p, std::int64_t position)
{
    MemoryStream* ms = static_cast<MemoryStream*>(p);
    ms->currentOffset = position;
    return rdfResultOk;
}

int MemoryStreamGetSize(void* p, std::int64_t* size)
{
    MemoryStream* ms = static_cast<MemoryStream*>(p);
    *size = ms->buffer.size();
    return rdfResultOk;
}
}

TEST_CASE("rdf::MemoryStream", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();

    SECTION("Basic read/write")
    {
        static const char buffer[] = "test";

        ms.Write(sizeof(buffer), buffer);
        CHECK(ms.Tell() == 5U);
        ms.Seek(0);

        char outputBuffer[5] = {};
        ms.Read(sizeof(outputBuffer), outputBuffer);

        CHECK(::strcmp(outputBuffer, "test") == 0);
    }
}

TEST_CASE("rdfUserStream", "[rdf]")
{
    struct Context
    {
        std::int64_t bytesRead = 0;
        std::int64_t bytesWritten = 0;

        bool isClosed = false;
    } ctx;

    rdfUserStream us = {};
    us.context = &ctx;

    us.Read = [](void* context, std::int64_t count, void* buffer, std::int64_t* bytesRead) -> int {
        static_cast<Context*>(context)->bytesRead += count;
        if (bytesRead) {
            *bytesRead = count;
        }

        return rdfResultOk;
    };

    us.Write = [](void* context,
                  std::int64_t count,
                  const void* buffer,
                  std::int64_t* bytesWritten) -> int {
        static_cast<Context*>(context)->bytesWritten += count;
        if (bytesWritten) {
            *bytesWritten = count;
        }

        return rdfResultOk;
    };

    us.Close = [](void* context) -> int {
        static_cast<Context*>(context)->isClosed = true;
        return rdfResultOk;
    };

    us.GetSize = [](void*, std::int64_t*) -> int { return rdfResultOk; };
    us.Tell = [](void*, std::int64_t*) -> int { return rdfResultOk; };
    us.Seek = [](void*, std::int64_t) -> int { return rdfResultOk; };

    rdfStream* stream = nullptr;
    rdfStreamCreateFromUserStream(&us, &stream);

    char buffer;
    rdfStreamRead(stream, 1024, &buffer, nullptr);
    CHECK(ctx.bytesRead == 1024);

    rdfStreamWrite(stream, 256, &buffer, nullptr);
    CHECK(ctx.bytesWritten == 256);

    rdfStreamClose(&stream);

    // Our close sets "isClosed"
    CHECK(ctx.isClosed);
}

TEST_CASE("ChunkFileWriter with write-only stream", "[rdf]")
{
    MemoryStream ms;
    rdfUserStream us = {};
    us.context = &ms;
    us.GetSize = MemoryStreamGetSize;
    us.Read = nullptr;
    us.Write = MemoryStreamWrite;
    us.Seek = MemoryStreamSeek;
    us.Tell = MemoryStreamTell;

    rdfStream* stream = nullptr;
    rdfStreamCreateFromUserStream(&us, &stream);

    rdfChunkFileWriter* chunkFileWriter = nullptr;
    rdfChunkFileWriterCreate(stream, &chunkFileWriter);

    rdfChunkCreateInfo chunkCreateInfo = {};
    chunkCreateInfo.headerSize = 0;
    ::memcpy(chunkCreateInfo.identifier, "FOO", 3);

    rdfChunkFileWriterWriteChunk(chunkFileWriter, &chunkCreateInfo, 0, nullptr, nullptr);

    rdfChunkFileWriterDestroy(&chunkFileWriter);

    CHECK(ms.buffer.size() > 0);

    rdfStreamClose(&stream);
}

TEST_CASE("rdfUserStream required functions", "[rdf]") 
{
    MemoryStream ms;
    rdfUserStream us = {};
    us.context = &ms;
    us.GetSize = MemoryStreamGetSize;
    us.Read = MemoryStreamRead;
    us.Write = MemoryStreamWrite;
    us.Seek = MemoryStreamSeek;
    us.Tell = MemoryStreamTell;

    SECTION("Write only stream works")
    {
        us.Read = nullptr;
        rdfStream* stream = nullptr;
        rdfStreamCreateFromUserStream(&us, &stream);
        SUCCEED();
        rdfStreamClose(&stream);
    }

    SECTION("Read only stream works")
    {
        us.Write = nullptr;
        rdfStream* stream = nullptr;
        rdfStreamCreateFromUserStream(&us, &stream);
        SUCCEED();
        rdfStreamClose(&stream);
    }

    SECTION("Read and write stream works")
    {
        rdfStream* stream = nullptr;
        rdfStreamCreateFromUserStream(&us, &stream);
        SUCCEED();
        rdfStreamClose(&stream);
    }

    SECTION("Fails if both read and write are null")
    {
        us.Read = nullptr;
        us.Write = nullptr;
        rdfStream* stream = nullptr;
        rdfStreamCreateFromUserStream(&us, &stream);
        CHECK(stream == nullptr);
    }

    SECTION("Seek must not be null")
    {
        us.Seek = nullptr;
        rdfStream* stream = nullptr;
        CHECK(rdfStreamCreateFromUserStream(&us, &stream) != rdfResultOk);
        CHECK(stream == nullptr);
    }

    SECTION("Tell must not be null")
    {
        us.Tell = nullptr;
        rdfStream* stream = nullptr;
        CHECK(rdfStreamCreateFromUserStream(&us, &stream) != rdfResultOk);
        CHECK(stream == nullptr);
    }

    SECTION("GetSize must not be null")
    {
        us.GetSize = nullptr;
        rdfStream* stream = nullptr;
        CHECK(rdfStreamCreateFromUserStream(&us, &stream) != rdfResultOk);
        CHECK(stream == nullptr);
    }
}

TEST_CASE("rdfUserStream Close() can throw", "[rdf]")
{
    /*
    Check that we can throw from Close(), and we can catch that, and still
    repeat the Close() and correctly close the stream.
    */

    bool shouldThrow = true;

    rdfUserStream us = {};
    us.GetSize = [](void* ctx, int64_t* o) -> int {
        return rdfResultOk;
    };
    us.Read = [](void* ctx, int64_t c, void* p, int64_t* o) -> int { 
        return rdfResultOk;
    };
    us.Write = [](void* ctx, int64_t c, const void* p, int64_t* o) -> int {
        return rdfResultOk;
    };
    us.Seek = [](void* ctx, int64_t o) -> int {
        return rdfResultOk;
    };
    us.Tell = [](void* ctx, int64_t* p) -> int {
        *p = 0;
        return rdfResultOk;
    };
    us.Close = [](void* ctx) -> int {
        if (*static_cast<bool*>(ctx)) {
            throw 23;
        }

        return rdfResultOk;
    };
    us.context = &shouldThrow;

    rdfStream* stream;
    rdfStreamCreateFromUserStream(&us, &stream);

    CHECK(rdfStreamClose(&stream) != rdfResultOk);
    REQUIRE(stream != nullptr);

    shouldThrow = false;
    CHECK(rdfStreamClose(&stream) == rdfResultOk);
    CHECK(stream == nullptr);
}
