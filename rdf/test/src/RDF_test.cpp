/* Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved. */
#include <catch2/catch.hpp>

#include "amdrdf.h"

#include <cstring>
#include "test_rdf.h"


TEST_CASE("rdf::LoadKnownGoodFile", "[rdf]")
{
    auto ms = rdf::Stream::FromReadOnlyMemory(test_rdf_len, test_rdf);

    rdf::ChunkFile cf(ms);

    CHECK(cf.ContainsChunk("chunk0"));
    CHECK(cf.ContainsChunk("chunk1"));
    CHECK(cf.ContainsChunk("chunk2"));

    CHECK(cf.ContainsChunk("chunk0", 0));
    CHECK(cf.ContainsChunk("chunk0", 1));
    CHECK(!cf.ContainsChunk("chunk0", 2));

    CHECK(!cf.ContainsChunk("chunk3"));

    cf.ReadChunkData("chunk0", 0, [](size_t size, const void* data) -> void {
        CHECK(std::string(
            static_cast<const char*>(data),
            static_cast<const char*>(data) + size) ==
            "some data");
    });

    CHECK(cf.GetChunkVersion("chunk1") == 3);
}

TEST_CASE("rdf::GetChunkVersion ignoring index bug", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();

    rdf::ChunkFileWriter writer(ms);
    writer.WriteChunk("chunk0", 0, nullptr, 0, nullptr, rdfCompressionNone, 1);
    writer.WriteChunk("chunk0", 0, nullptr, 0, nullptr, rdfCompressionNone, 2);
    writer.Close();

    rdf::ChunkFile cf(ms);
    CHECK(cf.GetChunkVersion("chunk0", 0) == 1);
    CHECK(cf.GetChunkVersion("chunk0", 1) == 2);
}

TEST_CASE("rdf::GetChunkCount", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();

    rdf::ChunkFileWriter writer(ms);
    writer.WriteChunk("chunk0", 0, nullptr, 0, nullptr, rdfCompressionNone, 1);
    writer.WriteChunk("chunk0", 0, nullptr, 0, nullptr, rdfCompressionNone, 2);
    writer.Close();

    rdf::ChunkFile cf(ms);
    CHECK(cf.GetChunkCount("chunk0") == 2);
    CHECK(cf.GetChunkCount("chunk1") == 0);
}

TEST_CASE("rdf::ChunkFileIterator", "[rdf]")
{
    auto ms = rdf::Stream::FromReadOnlyMemory(test_rdf_len, test_rdf);

    rdf::ChunkFile cf(ms);

    int chunkCount = 0;
    auto iterator = cf.GetIterator();
    while (!iterator.IsAtEnd()) {
        ++chunkCount;
        iterator.Advance();
    }

    CHECK(chunkCount == 4);
}

TEST_CASE("rdf::ChunkFileWriter general tests", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();
    rdf::ChunkFileWriter writer(ms);

    SECTION("Create chunk with maximum name length (WriteChunk)")
    {
        writer.WriteChunk("0123456789012345", 0, nullptr, 0, nullptr);
        writer.Close();

        rdf::ChunkFile cf(ms);
        CHECK(cf.GetChunkCount("0123456789012345") == 1);
    }

    SECTION("Create chunk with maximum name length (BeginChunk)")
    {
        writer.BeginChunk("0123456789012345", 0, nullptr);
        writer.EndChunk();
        writer.Close();

        rdf::ChunkFile cf(ms);
        CHECK(cf.GetChunkCount("0123456789012345") == 1);
    }

    SECTION("Create chunk returns index (WriteChunk)")
    {
        CHECK(writer.WriteChunk("chunk", 0, nullptr, 0, nullptr) == 0);
        CHECK(writer.WriteChunk("chunk", 0, nullptr, 0, nullptr) == 1);
        writer.Close();
    }
}

TEST_CASE("rdfChunkFileWriter general tests", "[rdf]")
{
    rdfStream* ms;
    rdfStreamCreateMemoryStream(&ms);

    rdfChunkFileWriter* writer;
    rdfChunkFileWriterCreate(ms, &writer);

    SECTION("Create chunk with maximum name length")
    {
        rdfChunkCreateInfo ci = {};
        ::memcpy(ci.identifier, "0123456789012345", 16);
        ci.version = 1;

        rdfChunkFileWriterBeginChunk(writer, &ci);

        int index0 = 0;
        rdfChunkFileWriterEndChunk(writer, &index0);

        CHECK(index0 == 0);

        rdfChunkFileWriterDestroy(&writer);
        writer = nullptr;

        rdfChunkFile* cf;
        rdfChunkFileOpenStream(ms, &cf);

        int64_t count = 0;
        rdfChunkFileGetChunkCount(cf, "0123456789012345", &count);
        CHECK(count == 1);

        rdfChunkFileClose(&cf);
    }

    SECTION("Create chunk with maximum name length (WriteChunk)")
    {
        rdfChunkCreateInfo ci = {};
        uint32_t headerData = 0xDEADBEEF;

        ci.pHeader = &headerData;
        ci.headerSize = sizeof(headerData);

        ::memcpy(ci.identifier, "0123456789012345", 16);
        ci.version = 1;

        int index0 = 0;
        rdfChunkFileWriterWriteChunk(writer, &ci, 0, nullptr, &index0);

        CHECK(index0 == 0);

        rdfChunkFileWriterDestroy(&writer);
        writer = nullptr;

        rdfChunkFile* cf;
        rdfChunkFileOpenStream(ms, &cf);

        int64_t count = 0;
        rdfChunkFileGetChunkCount(cf, "0123456789012345", &count);
        CHECK(count == 1);

        rdfChunkFileClose(&cf);
    }

    if (writer) {
        rdfChunkFileWriterDestroy(&writer);
    }

    rdfStreamClose(&ms);
}

TEST_CASE("rdf::ChunkFileWriter handling negative input sizes", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();
    rdf::ChunkFileWriter writer(ms);

    SECTION("Negative header size")
    {
        CHECK_THROWS_AS(writer.WriteChunk("chunk0", -1, nullptr, 0, nullptr), rdf::ApiException);
    }

    SECTION("Negative chunk size")
    {
        CHECK_THROWS_AS(writer.WriteChunk("chunk0", 0, nullptr, -1, nullptr), rdf::ApiException);
    }
}

TEST_CASE("rdf::ChunkFileWriter append", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();
    
    {
        rdf::ChunkFileWriter writer(ms);
        writer.WriteChunk("chunk0", 0, nullptr, 4, "Test");
        writer.Close();
    }

    {
        rdf::ChunkFileWriter writer(ms, rdf::ChunkFileWriteMode::Append);
        writer.WriteChunk("chunk1", 0, nullptr, 4, "Test");
        writer.Close();
    }

    rdf::ChunkFile cf(ms);
    CHECK(cf.ContainsChunk("chunk0"));
    CHECK(cf.ContainsChunk("chunk1"));
}

TEST_CASE("rdf::ChunkFileWriter append increments index", "[rdf]")
{
    auto ms = rdf::Stream::CreateMemoryStream();

    {
        rdf::ChunkFileWriter writer(ms);
        writer.WriteChunk("chunk", 0, nullptr, 4, "Test");
        writer.Close();
    }

    {
        rdf::ChunkFileWriter writer(ms, rdf::ChunkFileWriteMode::Append);
        int index = writer.WriteChunk("chunk", 0, nullptr, 4, "Test");
        CHECK(index == 1);
        writer.Close();
    }

    rdf::ChunkFile cf(ms);
    CHECK(cf.ContainsChunk("chunk", 0));
    CHECK(cf.ContainsChunk("chunk", 1));
}
