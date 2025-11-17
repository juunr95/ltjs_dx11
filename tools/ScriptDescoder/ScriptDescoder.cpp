#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "blowfish.h"

// Chave do CGameButeMgr (LithTech)
static UBYTE_08bits gKey[4] = { 27, 14, 21, 4 }; // m_szCryptKey sem o terminador

// Função para descriptografar um arquivo .txt criptografado com Blowfish
bool DecryptFile(const char* inputPath, const char* outputPath)
{
    std::ifstream fin(inputPath, std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << "[ERRO] Não foi possível abrir o arquivo de entrada: " << inputPath << "\n";
        return false;
    }

    std::ofstream fout(outputPath, std::ios::binary);
    if (!fout.is_open()) {
        std::cerr << "[ERRO] Não foi possível criar o arquivo de saída: " << outputPath << "\n";
        return false;
    }

    // Inicializa Blowfish
    InitializeBlowfish(gKey, sizeof(gKey));

    int n = 0;
    char buf[8];
    char tbuf[8];
    bool bFirstTime = true;

    while (!fin.eof()) {
        fin.read(buf, 8);
        n = static_cast<int>(fin.gcount());
        if (n == 1)
            n = (int)buf[0];
        if (!bFirstTime)
            fout.write(tbuf, n);
        else
            bFirstTime = false;
        Blowfish_decipher((UWORD_32bits*)buf, (UWORD_32bits*)&buf[4]);
        memcpy(tbuf, buf, 8);
    }

    fin.close();
    fout.close();

    std::cout << "[OK] Arquivo descriptografado salvo em: " << outputPath << "\n";
    return true;
}

// --------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::cout << "=== Blowfish TXT Decrypt Utility ===\n";

    if (argc < 3) {
        std::cout << "Uso: " << argv[0] << " <arquivo_entrada.txt> <arquivo_saida.txt>\n";
        return 1;
    }

    const char* inputPath = argv[1];
    const char* outputPath = argv[2];

    if (!DecryptFile(inputPath, outputPath)) {
        std::cerr << "Falha ao descriptografar o arquivo.\n";
        return 1;
    }

    return 0;
}
