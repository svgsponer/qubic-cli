#include <cstdint>
#include "keyUtils.h"
#include "K12AndKeyUtil.h"
bool getSubseedFromSeed(const uint8_t* seed, uint8_t* subseed)
{
    uint8_t seedBytes[55];
    for (int i = 0; i < 55; i++)
    {
        if (seed[i] < 'a' || seed[i] > 'z')
        {
            return false;
        }
        seedBytes[i] = seed[i] - 'a';
    }
    KangarooTwelve(seedBytes, sizeof(seedBytes), subseed, 32);

    return true;
}
void getPrivateKeyFromSubSeed(const uint8_t* seed, uint8_t* privateKey)
{
    KangarooTwelve(seed, 32, privateKey, 32);
}
void getPublicKeyFromPrivateKey(const uint8_t* privateKey, uint8_t* publicKey)
{
    point_t P;
    ecc_mul_fixed((unsigned long long*)privateKey, P); // Compute public key
    encode(P, publicKey);
}

void getIdentityFromPublicKey(const uint8_t* pubkey, char* dstIdentity, bool isLowerCase)
{
    uint8_t publicKey[32] ;
    memcpy(publicKey, pubkey, 32);
    uint16_t identity[61] = {0};
    for (int i = 0; i < 4; i++)
    {
        unsigned long long publicKeyFragment = *((unsigned long long*)&publicKey[i << 3]);
        for (int j = 0; j < 14; j++)
        {
            identity[i * 14 + j] = publicKeyFragment % 26 + (isLowerCase ? L'a' : L'A');
            publicKeyFragment /= 26;
        }
    }
    unsigned int identityBytesChecksum;
    KangarooTwelve(publicKey, 32, (uint8_t*)&identityBytesChecksum, 3);
    identityBytesChecksum &= 0x3FFFF;
    for (int i = 0; i < 4; i++)
    {
        identity[56 + i] = identityBytesChecksum % 26 + (isLowerCase ? L'a' : L'A');
        identityBytesChecksum /= 26;
    }
    identity[60] = 0;
    for (int i = 0; i < 60; i++) dstIdentity[i] = identity[i];
}
void getTxHashFromDigest(const uint8_t* digest, char* txHash)
{
    bool isLowerCase = true;
    getIdentityFromPublicKey(digest, txHash, isLowerCase);
}

void getPublicKeyFromIdentity(const char* identity, uint8_t* publicKey)
{
    unsigned char publicKeyBuffer[32];
    for (int i = 0; i < 4; i++)
    {
        *((unsigned long long*)&publicKeyBuffer[i << 3]) = 0;
        for (int j = 14; j-- > 0; )
        {
            if (identity[i * 14 + j] < 'A' || identity[i * 14 + j] > 'Z')
            {
                return;
            }

            *((unsigned long long*)&publicKeyBuffer[i << 3]) = *((unsigned long long*)&publicKeyBuffer[i << 3]) * 26 + (identity[i * 14 + j] - 'A');
        }
    }
    memcpy(publicKey, publicKeyBuffer, 32);
}