/*
 * Copyright 2020,2021. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로 제작되었습니다.
 */
#include <bsd/stdlib.h>
#include <string.h>
#include <gmp.h>
#include "rsa_pss.h"

#if defined(SHA224)
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha224;
#elif defined(SHA256)
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha256;
#elif defined(SHA384)
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha384;
#else
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha512;
#endif

#define HASH_S SHASIZE/8
#define RSA_S RSAKEYSIZE/8
#define DB_S (RSAKEYSIZE - SHASIZE - 8)/8
#define PAD2_S (DB_S - SHASIZE/8)



/*
 * Copyright 2020, 2021. Heekuck Oh, all rights reserved
 * rsa_generate_key() - generates RSA keys e, d and n in octet strings.
 * If mode = 0, then e = 65537 is used. Otherwise e will be randomly selected.
 * Carmichael's totient function Lambda(n) is used.
 */
void rsa_generate_key(void *_e, void *_d, void *_n, int mode)
{
    mpz_t p, q, lambda, e, d, n, gcd;
    gmp_randstate_t state;
    
    /*
     * Initialize mpz variables
     */
    mpz_inits(p, q, lambda, e, d, n, gcd, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, arc4random());
    /*
     * Generate prime p and q such that 2^(RSAKEYSIZE-1) <= p*q < 2^RSAKEYSIZE
     */
    do {
        do {
            mpz_urandomb(p, state, RSAKEYSIZE/2);
            mpz_setbit(p, 0);
            mpz_setbit(p, RSAKEYSIZE/2-1);
       } while (mpz_probab_prime_p(p, 50) == 0);
        do {
            mpz_urandomb(q, state, RSAKEYSIZE/2);
            mpz_setbit(q, 0);
            mpz_setbit(q, RSAKEYSIZE/2-1);
        } while (mpz_probab_prime_p(q, 50) == 0);
        mpz_mul(n, p, q);
    } while (!mpz_tstbit(n, RSAKEYSIZE-1));
    /*
     * Generate e and d using Lambda(n)
     */
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_lcm(lambda, p, q);
    if (mode == 0)
        mpz_set_ui(e, 65537);
    else do {
        mpz_urandomb(e, state, RSAKEYSIZE);
        mpz_gcd(gcd, e, lambda);
    } while (mpz_cmp(e, lambda) >= 0 || mpz_cmp_ui(gcd, 1) != 0);
    mpz_invert(d, e, lambda);
    /*
     * Convert mpz_t values into octet strings
     */
    mpz_export(_e, NULL, 1, RSAKEYSIZE/8, 1, 0, e);
    mpz_export(_d, NULL, 1, RSAKEYSIZE/8, 1, 0, d);
    mpz_export(_n, NULL, 1, RSAKEYSIZE/8, 1, 0, n);
    /*
     * Free the space occupied by mpz variables
     */
    mpz_clears(p, q, lambda, e, d, n, gcd, NULL);
}

/*
 * Copyright 2020. Heekuck Oh, all rights reserved
 * rsa_cipher() - compute m^k mod n
 * If m >= n then returns EM_MSG_OUT_OF_RANGE, otherwise returns 0 for success.
 */
static int rsa_cipher(void *_m, const void *_k, const void *_n)
{
    mpz_t m, k, n;
    
    /*
     * Initialize mpz variables
     */
    mpz_inits(m, k, n, NULL);
    /*
     * Convert big-endian octets into mpz_t values
     */
    mpz_import(m, RSAKEYSIZE/8, 1, 1, 1, 0, _m);
    mpz_import(k, RSAKEYSIZE/8, 1, 1, 1, 0, _k);
    mpz_import(n, RSAKEYSIZE/8, 1, 1, 1, 0, _n);
    /*
     * Compute m^k mod n
     */
    if (mpz_cmp(m, n) >= 0) {
        mpz_clears(m, k, n, NULL);
        return EM_MSG_OUT_OF_RANGE;
    }
    mpz_powm(m, m, k, n);
    /*
     * Convert mpz_t m into the octet string _m
     */
    mpz_export(_m, NULL, 1, RSAKEYSIZE/8, 1, 0, m);
    /*
     * Free the space occupied by mpz variables
     */
    mpz_clears(m, k, n, NULL);
    return 0;
}

/*
 * Copyright 2020. Heekuck Oh, all rights reserved
 * A mask generation function based on a hash function
 */
static unsigned char *mgf(const unsigned char *mgfSeed, size_t seedLen, unsigned char *mask, size_t maskLen)
{
    uint32_t i, count, c;
    size_t hLen;
    unsigned char *mgfIn, *m;
    
    /*
     * Check if maskLen > 2^32*hLen
     */
    hLen = SHASIZE/8;
    if (maskLen > 0x0100000000*hLen)
        return NULL;
    /*
     * Generate octet string mask
     */
    if ((mgfIn = (unsigned char *)malloc(seedLen+4)) == NULL)
        return NULL;;
    memcpy(mgfIn, mgfSeed, seedLen);
    count = maskLen/hLen + (maskLen%hLen ? 1 : 0);
    if ((m = (unsigned char *)malloc(count*hLen)) == NULL)
        return NULL;
    /*
     * Convert i to an octet string C of length 4 octets
     * Concatenate the hash of the seed mgfSeed and C to the octet string T:
     *       T = T || Hash(mgfSeed || C)
     */
    for (i = 0; i < count; i++) {
        c = i;
        mgfIn[seedLen+3] = c & 0x000000ff; c >>= 8;
        mgfIn[seedLen+2] = c & 0x000000ff; c >>= 8;
        mgfIn[seedLen+1] = c & 0x000000ff; c >>= 8;
        mgfIn[seedLen] = c & 0x000000ff;
        (*sha)(mgfIn, seedLen+4, m+i*hLen);
    }
    /*
     * Copy the mask and free memory
     */
    memcpy(mask, m, maskLen);
    free(mgfIn); free(m);
    return mask;
}

/*
 * rsassa_pss_sign - RSA Signature Scheme with Appendix
 */
int rsassa_pss_sign(const void *m, size_t mLen, const void *d, const void *n, void *s)
{
	unsigned char mhash[HASH_S];
	unsigned char salt[HASH_S];
	unsigned char M_prime[8 + HASH_S * 2];

	unsigned char H[HASH_S];
	unsigned char mgf_H[DB_S];

	unsigned char DB[DB_S];

	unsigned char maskedDB[DB_S];
	unsigned char EM[RSA_S];

	uint64_t is_long = 1;
	uint8_t bc = 0xbc;
	is_long = (is_long << 60) - 1;//2^61-1 (2^64bit = 2^61byte)


// --- error check ---
	if(m >= n)	return EM_MSG_OUT_OF_RANGE;//EM_MSG_OUT_OF_RANGE 메세지의 길이는 n보다 작아야함.

	if(mLen > is_long && SHASIZE <= 256)	return EM_MSG_TOO_LONG;//EM_MSG_TOO_LONG

	if(RSA_S < HASH_S * 2 + 2)	return EM_HASH_TOO_LONG; //EM_HASH_TOO_LONG

// --- M_prime --- 
	memset(M_prime, 0x00, 8);//padding1
	sha(m, mLen, mhash);	//mhash = hash(m)
	*salt = arc4random_uniform(SHASIZE);	//upperbound가 SHASIZE인 random # salt 생성
	memcpy(M_prime + 8, mhash, HASH_S); //M_prime = pad1 || mhash
	memcpy(M_prime + 8 + HASH_S, salt, HASH_S); // M_prime = pad1 || mhash || salt

// --- H ---
	sha(M_prime, 8 + HASH_S * 2, H);//H = hash(M_prime);

// --- DB ---
	
	memset(DB, 0x00, PAD2_S);//padding2
	DB[PAD2_S - 1] = 0x01; // padding2 : 0x01
	
	memcpy(DB + PAD2_S, salt, HASH_S); // DB = padding2 || salt

// --- maskedDB ---
	mgf(H, HASH_S, mgf_H, DB_S);//mgf_H = mgf(H)

	for(int i = 0; i < DB_S; i++)	maskedDB[i] = DB[i] ^ mgf_H[i];//maskedDB = DB ^ mgf_H;

	maskedDB[0] = 0x00;//MSB bit 0

// --- EM ---

	memcpy(EM, maskedDB, DB_S); //EM = masked_DB
	memcpy(EM + DB_S, H, HASH_S); //EM = maskedDB || H 
	memcpy(EM + RSA_S - 1, &bc,1); // EM = maskedDB || TF(0xbc)


// --- error check ---
	if(rsa_cipher(EM, d, n))	return EM_MSG_OUT_OF_RANGE; //EM cipher & EM_MSG_OUT_OF_RANGE

// ---
	memcpy(s, EM, RSA_S);//s = EM
	return 0;
}

/*
 * rsassa_pss_verify - RSA Signature Scheme with Appendix
 */
int rsassa_pss_verify(const void *m, size_t mLen, const void *e, const void *n, const void *s)
{
	unsigned char mhash[HASH_S];

	unsigned char EM[RSA_S];
	unsigned char maskedDB[DB_S];
	unsigned char H[HASH_S];
	unsigned char mgf_H[DB_S];

	unsigned char DB[DB_S];
	unsigned char salt[HASH_S];

	unsigned char M_prime[8 + HASH_S * 2]; 

	unsigned char H_prime[HASH_S];

	uint64_t is_long = 1;
	uint8_t bc = 0xbc;
	is_long = (is_long << 60) - 1;//2^61-1
	
	memcpy(EM, s, RSA_S);//EM = s

// --- error check ---
	if(m >= n)	return EM_MSG_OUT_OF_RANGE;//EM_MSG_OUT_OF_RANGE

	if(mLen > is_long)	return EM_MSG_TOO_LONG;//EM_MSG_TOO_LONG

	if(RSA_S < HASH_S * 2 + 2)	return EM_HASH_TOO_LONG; //EM_HASH_TOO_LONG

	if(rsa_cipher(EM, e, n))	return EM_MSG_OUT_OF_RANGE;//EM_MSG_OUT_OF_RANGE

	if(EM[RSAKEYSIZE/8 - 1] ^ bc)	return EM_INVALID_LAST;//EM_INVALID_LAST

	if(EM[0] >> 7 & 1)	return EM_INVALID_INIT;//EM_INVALID_INIT

// --- M ---
	sha(m,mLen,mhash); //mhash = hash(m) step2

// --- EM ---
	memcpy(maskedDB, EM, DB_S );// pick maskedDB from EM
	memcpy(H, EM + DB_S, HASH_S);// pick H from EM

	mgf(H, HASH_S , mgf_H, DB_S );//mgf_H = mgf(H)
	
// --- DB ---
	for(int i = 0; i < DB_S; i++)	DB[i] = maskedDB[i] ^ mgf_H[i];
	memcpy(salt, DB + PAD2_S , HASH_S);//pick salt from DB

	//-- check padding2 --
	for(int j = 1;j < PAD2_S - 1; j++){
		if(DB[j] & 1)	return EM_INVALID_PD2;//EM_INVALID_PD2
	}
		if(DB[PAD2_S - 1] != 0x01)	return EM_INVALID_PD2;//EM_INVALID_PD2

// --- M_prime ---
	memset(M_prime, 0x00, 8); //M_prime = padding1
	memcpy(M_prime + 8, mhash, HASH_S); //M_prime = padding1 || mhash
	memcpy(M_prime + 8 + HASH_S, salt, HASH_S); //M_prime = padding || mhash || salt

//H_prime
	sha(M_prime,8 + HASH_S * 2, H_prime); // H_prime = hash(M_prime)

// --- error7 check
	if(memcmp(H, H_prime, HASH_S)!=0)	return EM_HASH_MISMATCH;//compare H, H_prime

// ---
	return 0;
}
