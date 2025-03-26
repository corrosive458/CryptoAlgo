#include <iostream>
#include <string>
#include <random>
#include <ctime>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <bits/locale_facets_nonio.h>

#include "gost.h"
#include "BS_thread_pool.hpp"

#define strGenLength 65535 // 1073741824 // 1гб информации
#define BlockSize 8
#define KeySize 32

using namespace std;

uint8_t sBlocks[_GOST_TABLE_SIZE] = {
    0x09, 0x06, 0x03, 0x02, 0x08, 0x0B, 0x01, 0x07, 0x0A, 0x04, 0x0E, 0x0F, 0x0C, 0x00, 0x0D, 0x05,
    0x03, 0x07, 0x0E, 0x09, 0x08, 0x0A, 0x0F, 0x00, 0x05, 0x02, 0x06, 0x0C, 0x0B, 0x04, 0x0D, 0x01,
    0x0E, 0x04, 0x06, 0x02, 0x0B, 0x03, 0x0D, 0x08, 0x0C, 0x0F, 0x05, 0x0A, 0x00, 0x07, 0x01, 0x09,
    0x0E, 0x07, 0x0A, 0x0C, 0x0D, 0x01, 0x03, 0x09, 0x00, 0x02, 0x0B, 0x04, 0x0F, 0x08, 0x05, 0x06,
    0x0B, 0x05, 0x01, 0x09, 0x08, 0x0D, 0x0F, 0x00, 0x0E, 0x04, 0x02, 0x03, 0x0C, 0x07, 0x0A, 0x06,
    0x03, 0x0A, 0x0D, 0x0C, 0x01, 0x02, 0x00, 0x0B, 0x07, 0x05, 0x09, 0x04, 0x08, 0x0F, 0x0E, 0x06,
    0x01, 0x0D, 0x02, 0x09, 0x07, 0x0A, 0x06, 0x00, 0x08, 0x0C, 0x04, 0x05, 0x0F, 0x03, 0x0B, 0x0E,
    0x0B, 0x0A, 0x0F, 0x05, 0x00, 0x0C, 0x0E, 0x08, 0x06, 0x02, 0x03, 0x09, 0x01, 0x07, 0x0D, 0x04
};

void KeyIncr(uint8_t* key) {
    if (key[0] != 0xFF)
        key[0] += 1;
    else {
        for (int i = 1; i < KeySize; i++) {
            if (key[i] != 0xFF) {
                key[i] = key[i] + 1;
                memset(key, 0, i);
                return;
            }
        }
        memset(key, 0, KeySize);
    }
}

void AddToKey(uint8_t* key, int value) {
    int next = 0;
    while (value && next < KeySize) {
        int sum = key[next] + (value % 0x100);
        int temp = sum / 0x100;
        key[next] = sum % (0x100);
        value = (value / 0x100) + temp;
        next++;
    }
}

void encrypt(uint8_t* block, uint8_t* key) {
    GOST_Encrypt_SR(block, BlockSize, _GOST_Mode_Encrypt, sBlocks, key);
    delete[] key;
}

void decrypt(uint8_t* block, uint8_t* key) {
    GOST_Encrypt_SR(block, BlockSize, _GOST_Mode_Decrypt, sBlocks, key);
    delete[] key;
}

void encrypt_threads(uint8_t** TextBlocks, int start_i, int end_i, uint8_t* key) {
    for (int i = start_i; i < end_i; i++) {
        GOST_Encrypt_SR(TextBlocks[i], BlockSize, _GOST_Mode_Encrypt, sBlocks, key);
        KeyIncr(key);
    }
    delete[] key;
}

void decrypt_threads(uint8_t** TextBlocks, int start_i, int end_i, uint8_t* key) {
    for (int i = start_i; i < end_i; i++) {
        GOST_Encrypt_SR(TextBlocks[i], BlockSize, _GOST_Mode_Decrypt, sBlocks, key);
        KeyIncr(key);
    }
    delete[] key;
}

void PrintBlock(uint8_t* block) {
    for (int i = 0; i < BlockSize; i++) {
        cout << char(block[i]);
    }
    cout << endl;
}

void PrintBlockHex(uint8_t* block) {
    for (int i = 0; i < BlockSize; i++) {
        if (block[i] < 16)
            cout << hex << uppercase << "0x0" << int(block[i]) << ' ';
        else
            cout << hex << uppercase << "0x" << int(block[i]) << ' ';
    }
    cout << endl;
}

void KeyGen(uint8_t* key) {
    uint8_t templ[85] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '!', '@', '#', '$', '%', '6', '7', '8', '9', ')', '_', '+', '"', ';', ':', '?', '*', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '/', '\\', '|', '<', '>', '~'};
    srand(time(NULL));
    for (int i = 0; i < KeySize; i++) {
        key[i] = templ[rand() % 85];
    }
}

void strGen(uint8_t* key) {
    uint8_t templ[85] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '!', '@', '#', '$', '%', '6', '7', '8', '9', ')', '_', '+', '"', ';', ':', '?', '*', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '/', '\\', '|', '<', '>', '~'};
    srand(time(NULL));
    for (int i = 0; i < strGenLength; i++) {
        key[i] = templ[rand() % 85];
    }
}


int main()
{
    uint8_t key[KeySize] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    //  ПОЛУЧЕНИЕ КЛЮЧА

    cout << "Введите ключ длиной " << KeySize << " символов или пустую строку для генерации ключа: ";
    string temp_key;
    getline(cin, temp_key);

    if (temp_key == "") {
        KeyGen(key);
        cout << "Ваш ключ: ";
        for (int i = 0; i < KeySize; i++)
            cout << key[i];
        cout << endl;
    }
    else {
        for (int i = 0; i < temp_key.length() && i < KeySize; i++)
            key[KeySize - 1 - i] = temp_key[i];
    }

    //  ПОЛУЧЕНИЕ ОТКРЫТОГО ТЕКСТА

    cout << "Введите открытый текст или пустую строку для генерации случайного текста: ";
    string str;
    getline(cin, str);

    int text_size;
    uint8_t* text;
    if (str == "") {
        text = new uint8_t[strGenLength];
        text_size = strGenLength;
        strGen(text);
    }
    else {
        text_size = str.length();
        text = new uint8_t[str.length()];
        for (int i = 0; i < text_size; i++)
            text[i] = str[i];
    }

    // РАЗБИТИЕ ТЕКСТА НА БЛОКИ

    int block_count = (text_size + BlockSize - 1) / BlockSize;
    uint8_t** TextBlocks = new uint8_t*[block_count];

    int k = 0;
    for (int i = 0; i < text_size; i += BlockSize) {
        uint8_t *temp = new uint8_t[BlockSize];
        memset(temp, 0, BlockSize);

        memcpy(temp, &text[i], (text_size - i < BlockSize) ? (text_size - i) : BlockSize);
        TextBlocks[k] = temp;
        k++;
    }

    if (strGenLength <= 65535) {
        cout << endl << "Исходный текст (по блокам) и его представление в шестнадцатеричной системе" << endl;
        for (int i = 0; i < block_count; i++)
            PrintBlock(TextBlocks[i]);
        for (int i = 0; i < block_count; i++)
            PrintBlockHex(TextBlocks[i]);
    }

    cout << "Введите количество потоков или 0 для автоматического определения: " << endl;
    int threads_count;
    cin >> threads_count;
    if (threads_count == 0)
        threads_count = int(thread::hardware_concurrency());

    BS::thread_pool pool(threads_count);


    // ПРОЦЕСС ЗАШИФРОВАНИЯ

    int chunk_size = block_count / threads_count;
    auto begin = std::chrono::steady_clock::now();

    for (int i = 0; i < block_count; i += chunk_size) {
        std::future<void> my_future = pool.submit_task(
        [TextBlocks, i, block_count, chunk_size, key]()
        {
            uint8_t* temp = new uint8_t[KeySize];
            memcpy(temp, key, KeySize);
            AddToKey(temp, i);
            encrypt_threads(TextBlocks, i, i + chunk_size > block_count ? block_count : i + chunk_size, temp);
        });
    }

    pool.wait();

    auto end = std::chrono::steady_clock::now();

    if (strGenLength <= 65535) {
        cout << "Шифротекст в шестнадцатеричной системе" << endl;
        for (int i = 0; i < block_count; i++)
            PrintBlockHex(TextBlocks[i]);
    }

    //  ПРОЦЕСС РАСШИФРОВАНИЯ

    auto begin2 = std::chrono::steady_clock::now();

    for (int i = 0; i < block_count; i += chunk_size) {
        std::future<void> my_future = pool.submit_task(
        [TextBlocks, i, block_count, chunk_size, key]()
        {
            uint8_t* temp = new uint8_t[KeySize];
            memcpy(temp, key, KeySize);
            AddToKey(temp, i);
            decrypt_threads(TextBlocks, i, i + chunk_size > block_count ? block_count : i + chunk_size, temp);
        });
    }

    pool.wait();

    auto end2 = std::chrono::steady_clock::now();

    if (strGenLength <= 65535) {
        cout << "Расшифрованный текст (по блокам) и его представление в шестнадцатеричной системе:" << endl;
        for (int i = 0; i < block_count; i++)
            PrintBlock(TextBlocks[i]);
        for (int i = 0; i < block_count; i++)
            PrintBlockHex(TextBlocks[i]);
    }

    auto elapsed_ns = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    auto elapsed_ns2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2- begin2);

    cout << dec << "Время зашифрования: " << elapsed_ns.count() << " milli s\n";
    cout << dec << "Время расшифрования: "<< elapsed_ns2.count() << " milli s\n";
    cout << dec << "Общее время: "<< elapsed_ns.count() + elapsed_ns2.count() << " milli s\n";

    for (int i = 0; i < block_count; i++) {
        delete[] TextBlocks[i];
    }

    delete[] text;
    delete[] TextBlocks;

}

/* ??? ??? ????????????
int main(int argc, char* argv[]) {
    if (argc > 1) {
        int all_time = 0;
        for (int i = 0; i < atoi(argv[1]); i++) {
            all_time += test();
        }
        cout << int(all_time/atoi(argv[1])) << endl;
    } else {
        int all_time = 0;
        for (int i = 0; i < 1000; i++) {
            all_time += test();
        }
        cout << int(all_time / 1000) << endl;
    }

}*/
