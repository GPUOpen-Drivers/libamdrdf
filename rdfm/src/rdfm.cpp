/* Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. */
#include <cli11/CLI11.hpp>
#include <json/json.hpp>

#include "amdrdf.h"

#include <cstddef>
#include <algorithm>
#include <set>

namespace
{
std::set<std::string> GetChunkIdentifiers(rdf::ChunkFile& cf)
{
    std::set<std::string> chunkIds;

    auto it = cf.GetIterator();

    for (;;) {
        if (it.IsAtEnd()) {
            break;
        }

        // +1 so we get a trailing \0
        char name[RDF_IDENTIFIER_SIZE + 1] = {};
        it.GetChunkIdentifier(name);

        const std::string s(name);
        if (chunkIds.find(s) == chunkIds.end()) {
            chunkIds.insert(s);
        }

        it.Advance();
    }

    return chunkIds;
}

void CopyChunks(rdf::ChunkFile& cf, rdf::ChunkFileWriter& output, const bool compress)
{
    std::vector<std::byte> headerBuffer, dataBuffer;
    auto it = cf.GetIterator();

    for (;;) {
        if (it.IsAtEnd()) {
            break;
        }

        char id[RDF_IDENTIFIER_SIZE + 1] = {};
        it.GetChunkIdentifier(id);

        const auto index = it.GetChunkIndex();
        const auto version = cf.GetChunkVersion(id, index);

        const auto chunkHeaderSize = cf.GetChunkHeaderSize(id, index);
        headerBuffer.resize(chunkHeaderSize);
        cf.ReadChunkHeaderToBuffer(id, index, headerBuffer.data());

        const auto chunkDataSize = cf.GetChunkDataSize(id, index);
        dataBuffer.resize(chunkDataSize);
        cf.ReadChunkDataToBuffer(id, index, dataBuffer.data());

        output.WriteChunk(id,
                          chunkHeaderSize,
                          headerBuffer.data(),
                          dataBuffer.size(),
                          dataBuffer.data(),
                          compress ? rdfCompressionZstd : rdfCompressionNone);

        it.Advance();
    }
}

int MergeChunkFiles(const std::string& input1,
                    const std::string& input2,
                    const std::string& output,
                    const bool compress)
{
    rdf::ChunkFile chunkFile1(input1.c_str());
    rdf::ChunkFile chunkFile2(input2.c_str());

    const auto file1Ids = GetChunkIdentifiers(chunkFile1);
    const auto file2Ids = GetChunkIdentifiers(chunkFile2);

    std::vector<std::string> intersection;
    std::set_intersection(file1Ids.begin(),
                          file1Ids.end(),
                          file2Ids.begin(),
                          file2Ids.end(),
                          std::back_inserter(intersection));

    if (!intersection.empty()) {
        std::cerr << "Cannot merge files containing the same chunk identifiers." << std::endl;

        return 1;
    }

    rdf::Stream outputFile = rdf::Stream::OpenFile(output.c_str());
    rdf::ChunkFileWriter chunkFileWriter(outputFile);

    CopyChunks(chunkFile1, chunkFileWriter, compress);
    CopyChunks(chunkFile2, chunkFileWriter, compress);

    // Must close before the output file goes out of scope
    chunkFileWriter.Close();

    return 0;
}
}  // namespace

int main(int argc, char* argv[])
{
    CLI::App app{"RDFM 1.0"};

    std::string input1, input2, output;
    bool compress = false;

    auto mergeCommand = app.add_subcommand("merge", "Merge two chunk files.");
    mergeCommand->add_option("input1", input1)->required();
    mergeCommand->add_option("input2", input2)->required();
    mergeCommand->add_option("output", output)->required();
    mergeCommand->add_flag("-c,--compress", compress);

    CLI11_PARSE(app, argc, argv);

    try {
        if (*mergeCommand) {
            return MergeChunkFiles(input1, input2, output, compress);
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}