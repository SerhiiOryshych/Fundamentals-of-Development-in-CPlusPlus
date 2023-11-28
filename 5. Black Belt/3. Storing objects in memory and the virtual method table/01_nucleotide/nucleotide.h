#include <cstddef>

struct Nucleotide {
    char Symbol;
    size_t Position;
    int ChromosomeNum;
    int GeneNum;
    bool IsMarked;
    char ServiceInfo;
};


struct CompactNucleotide {
    std::uint64_t Symbol: 2;
    std::uint64_t Position: 32;
    std::uint64_t ChromosomeNum: 6;
    std::uint64_t GeneNum: 15;
    std::uint64_t IsMarked: 1;
    std::uint64_t ServiceInfo: 8;
};


bool operator==(const Nucleotide &lhs, const Nucleotide &rhs) {
    return (lhs.Symbol == rhs.Symbol)
           && (lhs.Position == rhs.Position)
           && (lhs.ChromosomeNum == rhs.ChromosomeNum)
           && (lhs.GeneNum == rhs.GeneNum)
           && (lhs.IsMarked == rhs.IsMarked)
           && (lhs.ServiceInfo == rhs.ServiceInfo);
}

const std::map<char, int> symbol_to_idx = {{'A', 0},
                                           {'T', 1},
                                           {'G', 2},
                                           {'C', 3}};
const std::map<int, char> idx_to_symbol = {{0, 'A'},
                                           {1, 'T'},
                                           {2, 'G'},
                                           {3, 'C'}};

CompactNucleotide Compress(const Nucleotide &n) {
    CompactNucleotide cn;
    cn.Symbol = symbol_to_idx.at(n.Symbol);
    cn.Position = n.Position;
    cn.ChromosomeNum = n.ChromosomeNum;
    cn.GeneNum = n.GeneNum;
    cn.IsMarked = n.IsMarked ? 1 : 0;
    cn.ServiceInfo = (std::uint64_t) n.ServiceInfo;
    return cn;
};


Nucleotide Decompress(const CompactNucleotide &cn) {
    Nucleotide n;
    n.Symbol = idx_to_symbol.at(cn.Symbol);
    n.Position = cn.Position;
    n.ChromosomeNum = cn.ChromosomeNum;
    n.GeneNum = cn.GeneNum;
    n.IsMarked = cn.IsMarked == 1;
    n.ServiceInfo = (char) cn.ServiceInfo;
    return n;
}
