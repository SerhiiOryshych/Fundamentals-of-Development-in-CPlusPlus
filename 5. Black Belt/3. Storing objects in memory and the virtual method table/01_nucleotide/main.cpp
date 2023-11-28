#include "test_runner.h"
#include "nucleotide.h"

void TestSize() {
    ASSERT(sizeof(CompactNucleotide) <= 8);
}

void TestCompressDecompress() {
    Nucleotide source;

    source.Symbol = 'T';
    source.Position = 3'300'000'000;
    source.ChromosomeNum = 48;
    source.GeneNum = 25'000;
    source.IsMarked = true;
    source.ServiceInfo = 'Test';

    CompactNucleotide compressedSource = Compress(source);
    Nucleotide decompressedSource = Decompress(compressedSource);

    ASSERT(source == decompressedSource);
}

int main() {
    TestRunner tr;

    RUN_TEST(tr, TestSize);
    RUN_TEST(tr, TestCompressDecompress);

    return 0;
}
