/*
 * Copyright 2020, 2021. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로 제작되었습니다.
 */
#include <stdlib.h>
#include "mRSA.h"
#include <stdio.h>

/*
 * mRSA_generate_key() - generates mini RSA keys e, d and n
 * Carmichael's totient function Lambda(n) is used.
 */

const uint64_t a[ALEN] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

static uint64_t gcd(uint64_t a, uint64_t b)
{ 
    uint64_t tmp;
    while (b != 0) 
    {     
            tmp = a % b;
            a = b;
            b = tmp;
	}
        return a;//b == 0 일 때 a값이 최대공약수 
}

static uint64_t mul_inv(uint64_t a, uint64_t m)
{
    uint64_t d0 = a, d1 = m;
    uint64_t x0 = 1, x1 = 0;
    uint64_t q, tmp;
    int sign = -1;

    while (d1 > 1)
    {
        q = d0 / d1;

        tmp = d0 - q * d1; // d(n+2) = d(n) - d(n)/d(n+1) * d(n+1)
        d0 = d1;
        d1 = tmp;

        tmp = x0 + q * x1; // x(n+2) = x(n) - d(n)/d(n+1) * d(n+1)
        x0 = x1;
        x1 = tmp;

        sign = ~sign; //x1값이 while문을 돌 때마다 부호가 바뀜 unsign int를 고려함.
    }
    if (d1 == 1)
        return (sign ? m - x1 : x1); // sign변수를 통해 부호 결정.
    else
        return 0;
}

static uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m)
{
    a = a % m; //a modulo m
    b = b % m; //b modulo m
	return (a >= (m-b) ? a-(m-b) : a + b); // (a + b) >= m ? a + b - m : a + b)
}

static uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m)
{
    uint64_t r = 0;//
    while (b > 0)
    {
        if (b & 1)
            r = mod_add(r, a, m); //b의 비트가 1이라면 결과에 a를 더해줌.
        b = b >> 1;
        a = mod_add(a, a, m); // binary b << 1 에 해당하는 a값은 a + a 에 해당함
    }
    return r;
}

static uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m)
{
	uint64_t r = 1;
    while (b > 0)
    {
        if (b & 1)
            r = mod_mul(r, a, m); //b의 비트가 1일때 r에 a를 곱해줌.
        b = b >> 1;
        a = mod_mul(a, a, m); //binary b << 1 에 해당하는 a값은 a * a에 해당함.
	}
    return r;
}

static int miller_rabin(uint64_t n)
{
    uint64_t q, k, tmp;
    int is_prime = 0;
    int index = 0;

    if (n == 2 || n == 3)	return PRIME;// 2는 짝수지만 prime, 3은 while값에 들어갈 수 없으므로 미리 예외처리한다.
	k = 0;
    q = n - 1;
    while (q % 2 == 0)
    {
        q = q / 2;
        k++;
    } //q,k구하기
    while (a[index] < (n-1) && index < 12)
    {
        is_prime = COMPOSITE; //소수 판별 초기화
        if (mod_pow(a[index], q, n) == 1)
            is_prime = PRIME;
        else
        {
            for (int j = 0; j < k; j++)
            {
                if (j == 0)
                    tmp = mod_pow(a[index], q, n); //a^q mod n 저장
                else
                    tmp = mod_mul(tmp, tmp, n); //q(t+1) = q(t)*q(t) mod n
                if (tmp == n - 1)
                    is_prime = PRIME;
            }
        }
        if (is_prime == COMPOSITE)
            return COMPOSITE;//확정적인 합성수임.
        index++;
    }
    return is_prime;//while문을 모두 통과하면 99.99...%확률로 prime임.
}

void mRSA_generate_key(uint64_t *e, uint64_t *d, uint64_t *n)
{
    uint64_t p, q, tmp, theta; 
	uint64_t fbit = 1;


	fbit = (fbit<<31) + 1;//p와 q가 확정적으로 32비트로 만들기 위함
	tmp = 0;//p*q값 초기화
	
	while(tmp < MINIMUM_N){//n은 ninimun_n보다 커야 64비트를 만족함.
		p = fbit | arc4random(); //32th = 1, 1st = 1 고정, fit or arc4random()
		while (miller_rabin(p) == COMPOSITE)	p = fbit | arc4random();// prime p 찾기		
    	q = fbit | arc4random(); //32th = 1, 1st = 1 고정, fit or arc4random()
		while (miller_rabin(q) == COMPOSITE)	q = fbit | arc4random();// prime q 찾기
		tmp = p * q;
	}
	*n = tmp;

	theta = (p-1) * (q-1)/gcd(p-1,q-1);//계산의 양을 줄이기 위해 lcm 최소공약수 이용

	tmp = arc4random_uniform(theta);//theta보다 작은 e값 선정
	while(gcd(theta,tmp) != 1 || tmp <= 1)	tmp = arc4random_uniform(theta);//e값의 조건을 만족할때까지.
	*e = tmp;
	*d = mul_inv(*e,theta);
}

/*
 * mRSA_cipher() - compute m^k mod n
 * If data >= n then returns 1 (error), otherwise 0 (success).
 */
int mRSA_cipher(uint64_t *m, uint64_t k, uint64_t n)
{
	*m = mod_pow(*m, k, n);// m^k mod n
	if(*m >= n)	return 1;// data >= n then error
	else return 0; // success
}

