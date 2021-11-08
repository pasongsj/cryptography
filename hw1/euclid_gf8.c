/*
 * Copyright 2020, 2021. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로 제작되었습니다.
 */
#include <stdio.h>
#include <bsd/stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
/*
 * gcd() - Euclidean algorithm
 *
 * 유클리드 알고리즘 gcd(a,b) = gcd(b,a mod b)를 사용하여 최대공약수를 계산한다.
 * 만일 a가 0이면 b가 최대공약수가 된다. 그 반대도 마찬가지이다.
 * a, b가 모두 음이 아닌 정수라고 가정한다.
 * 재귀함수 호출을 사용하지 말고 while 루프를 사용하여 구현하는 것이 빠르고 좋다.
 */
int gcd(int a, int b)
{
	int tmp1;
	while(b != 0)//  b == 0일 때 나누어 떨어진 경우이기 때문에 a가 최대공약수가 됨. 
	{
		tmp1 =a%b;
		a = b;
		b = tmp1;
	}
	return a;	
    // 여기를 완성하세요
}

/*
 * xgcd() - Extended Euclidean algorithm
 *
 * 확장유클리드 알고리즘은 두 수의 최대공약수와 gcd(a,b) = ax + by 식을 만족하는
 * x와 y를 계산하는 알고리즘이다. 강의노트를 참조하여 구현한다.
 * a, b가 모두 음이 아닌 정수라고 가정한다.
 */
int xgcd(int a, int b, int *x, int *y) //x와 y값을 가져오기 위해 포인터를 잘 확인 해야함.
{
	int x0,x1,y0,y1,d0,d1;
	int q,tmp2;
	x0=y1=1;// x0 = 1, y1 = 1
	x1=y0=0;// x1 = 0, y0 = 0
	d0 = a;
	d1 = b;
	while(d1!=0)
	{
		//gcd(a,b) = a * x1 + b * y1을 만족하는 x1과 y1값을 구함
		q = d0/d1;

		tmp2 = x0 - q*x1;//x[i+1] = x[i-1] - q[i]*x[i]
		x0 = x1;
		x1 = tmp2;

		tmp2 = y0 - q*y1;//y[i+1] = y[i-1] - q[i]*y[i]
		y0 = y1;
		y1 = tmp2;

		tmp2 = d0 - q*d1;//d[i+1] = d[i-1] - q[i]*d[i]
		d0 = d1;
		d1 = tmp2;
	}	
	*x = x0;
	*y = y0;
	return d0;	
    // 여기를 완성하세요
}

/*
 * mul_inv() - computes multiplicative inverse a^-1 mod m
 *
 * 모듈로 m에서 a의 곱의 역인 a^-1 mod m을 구한다.
 * 만일 역이 존재하지 않는다면 0을 리턴해야 한다.
 * 확장유클리드 알고리즘을 변형하면 구할 수 있다. 강의노트를 참조한다.
 */
int mul_inv(int a, int m)
{
	int d0,d1,x0,x1,q,tmp3;
	d0 = a;
	d1 = m;
	x0 = 1;
	x1 = 0;
	while(d1>1)
	{
		q = d0/d1;
		
		tmp3 = d0 - q*d1;//변형된 확장유클리드 알고리즘과 swap(d0,d1)
		d0 = d1;
		d1 = tmp3;	
	
		tmp3 =x0 - q*x1;
		x0 = x1;
		x1 = tmp3;
	}
	if(d1 == 1)	return(x1>0 ? x1 : x1+m);
	else return 0;


    // 여기를 완성하세요
}

/*
 * umul_inv() - computes multiplicative inverse a^-1 mod m
 *
 * 입력이 unsigned 64 비트 정수로 되어 있는 특수한 경우에
 * 모듈로 m에서 a의 곱의 역인 a^-1 mod m을 구한다.
 * 만일 역이 존재하지 않는다면 0을 리턴해야 한다.
 * 확장유클리드 알고리즘을 변형하면 구할 수 있다.
 * 입출력 모두가 unsigned 64 비트 정수임을 고려해서 구현한다.
 */
 
uint64_t umul_inv(uint64_t a, uint64_t m) // unsigned 64 비트 정수에서 mul_inv와 같은 방식을 사용하여 a^-1 mod m을 구함
{
	uint64_t d0,d1,x0,x1,q,tmp4;
	d0 = a;
	d1 = m;
	x0 = 1;
	x1 = 0;
	while(d1>1)
	{
		q = d0/d1;

		tmp4 = d0 - q*d1;
		d0 = d1;
		d1 = tmp4;

		tmp4 = x0 - q*x1;
		x0 = x1;
		x1 = tmp4;
	}
	if(d1 == 1)	return((x1>>63)==0 ? x1 : x1+m);//첫번째 비트를 통해 부호를 확인함.
	else	return 0;

    // 여기를 완성하세요
}

/*
 * gf8_mul(a, b) - a * b mod x^8+x^4+x^3+x+1
 *
 * 7차식 다항식 a와 b를 곱하고 결과를 8차식 x^8+x^4+x^3+x+1로 나눈 나머지를 계산한다.
 * x^8 = x^4+x^3+x+1 (mod x^8+x^4+x^3+x+1) 특성을 이용한다.
 */

uint8_t gf8_mul(uint8_t a, uint8_t b)
{
	uint8_t res = 0;
	while(b > 0)
	{
		if(b & 1)	res = res ^ a; // b가 1일때 a값이 존재하므로 res와 XOR해준다.
		b = b >> 1; // b를 한비트씩 줄여줌
		a = ((a<<1)^((a>>7) & 1 ? 0X1B : 0));// a가 7차식인경우 atimes를 통해 ax로 만들어준다.
		//x^8 = x^4+x^3+x+1 (mod x^8+x^4+x^3+x+1) 특성을 이용하여 x^4+x^3+x+1 = 0X1B를 XOR해준다.
	}
	return res;
    // 여기를 완성하세요
}



/*
 * gf8_pow(a,b) - a^b mod x^8+x^4+x^3+x+1
 *
 * 7차식 다항식 a을 b번 지수승한 결과를 8차식 x^8+x^4+x^3+x+1로 나눈 나머지를 계산한다.
 * gf8_mul()과 "Square Multiplication" 알고리즘을 사용하여 구현한다.
 */

uint8_t gf8_pow(uint8_t a, uint8_t b) // project#1.pdf를 참고하여 일반적인 square multiplication을 이해하였다.
{
	uint8_t res = 1;
	while(b>0)
	{
		if(b & 1) res = gf8_mul(res,a);	//예) b = x^7+x^4+x^3+x+1일 때,
						//b = 10011011(B), 	result = result * a^(2^0),
						//b = 1001101(B), 	result = result * a^(2^1),
						//b = 10011(B), 	result = result * a^(2^3),
						//b = 1001(B), 	result = result * a^(2^4),
						//b = 1(B), 		result = result * a^(2^7),
						//b = 2^n,	result = result * a^(2^n)

		a = gf8_mul(a,a);
		//a^(2^n) * a^(2^n) = a^(2^(n+1)) , gf8_mul을 이용하여 a*a mod x^8+x^4+x^3+x+1를 구함
		b = b >> 1;
	}
	return res;
    // 여기를 완성하세요
}

/*
 * gf8_inv(a) - a^-1 mod x^8+x^4+x^3+x+1
 *
 * 모둘러 x^8+x^4+x^3+x+1에서 a의 역을 구한다.
 * 역을 구하는 가장 효율적인 방법은 다항식 확장유클리드 알고리즘을 사용하는 것이다.
 * 다만 여기서는 복잡성을 피하기 위해 느리지만 알기 쉬운 지수를 사용하여 구현하였다.
 */

uint8_t gf8_inv(uint8_t a)
{
    return gf8_pow(a, 0xfe);
}

/*
 * 함수가 올르게 동작하는지 검증하기 위한 메인 함수로 수정해서는 안 된다.
 */
int main(void)
{
    int a, b, x, y, d, count;
    uint64_t m, ai;
    
    /*
     * 기본 gcd 시험
     */
    printf("--- 기본 gcd 시험 ---\n");
    a = 28; b = 0;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    a = 0; b = 32;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    a = 41370; b = 22386;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    a = 22386; b = 41371;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));

    /*
     * 기본 xgcd, mul_inv 시험
     */
    printf("--- 기본 xgcd, mul_inv 시험 ---\n");
    a = 41370; b = 22386;
    d = xgcd(a, b, &x, &y);
    printf("%d = %d * %d + %d * %d\n", d, a, x, b, y);
    printf("%d^-1 mod %d = %d, %d^-1 mod %d = %d\n", a, b, mul_inv(a,b), b, a, mul_inv(b,a));
    a = 41371; b = 22386;
    d = xgcd(a, b, &x, &y);
    printf("%d = %d * %d + %d * %d\n", d, a, x, b, y);
    printf("%d^-1 mod %d = %d, %d^-1 mod %d = %d\n", a, b, mul_inv(a,b), b, a, mul_inv(b,a));

    /*
     * 난수 a와 b를 발생시켜 xgcd를 계산하고, 최대공약수가 1이면 역이 존재하므로
     * 여기서 얻은 a^-1 mod b와 b^-1 mod a를 mul_inv를 통해 확인한다.
     * 이 과정을 8백만번 이상 반복하여 올바른지 확인한다.
     */

    printf("--- 무작위 mul_inv 시험 ---\n"); fflush(stdout);
    count = 0;
    do {
        arc4random_buf(&a, sizeof(int)); a &= 0x7fffffff;
        arc4random_buf(&b, sizeof(int)); b &= 0x7fffffff;


        d = xgcd(a, b, &x, &y);
	if (d == 1) {
            if (x < 0)
                x = x + b;
            else
                y = y + a;
            if (x != mul_inv(a, b) || y != mul_inv(b, a)) {
                printf("Inversion error\n");
                exit(1);
            }
        }
        if (++count % 0xffff == 0) {
            printf(".");
            fflush(stdout);
        }
    } while (count < 0xfffff);
    printf("No error found\n");
  
    /*
     * GF(2^8)에서 기본 a*b 시험
     */
    printf("--- GF(2^8)에서 기본 a*b 시험 ---\n");
    a = 28; b = 7;
    printf("%d * %d = %d\n", a, b, gf8_mul(a,b));
    a = 127; b = 68;
    printf("%d * %d = %d\n", a, b, gf8_mul(a,b));

    /*
     * GF(2^8)에서 a를 1부터 255까지 a^-1를 구하고 a * a^-1 = 1인지 확인한다.
     */

    printf("--- GF(2^8)에서 전체 a*b 시험 ---\n");
    for (a = 1; a < 256; ++a) {
        if (a == 0) continue;
        b = gf8_inv(a);
        if (gf8_mul(a,b) != 1) {
            printf("Logic error\n");
            exit(1);
        }
        else {
            printf(".");
            fflush(stdout);
        }
    }
    printf("No error found\n");


    /*
     * umul_inv 시험
     */

    printf("--- 기본 umul_inv 시험 ---\n");
    a = 5; m = 9223372036854775808u;
    ai = umul_inv(a, m);
    printf("a = %d, m = %"PRIu64", a^-1 mod m = %"PRIu64"", a, m, ai);
    if (ai != 5534023222112865485u) {
        printf(" <- inversion error\n");
        exit(1);
    }
    else
        printf(" OK\n");
    a = 17; m = 9223372036854775808u;
    ai = umul_inv(a, m);
    printf("a = %d, m = %"PRIu64", a^-1 mod m = %"PRIu64"", a, m, ai);
    if (ai != 8138269444283625713u) {
        printf(" <- inversion error\n");
        exit(1);
    }
    else
        printf(" OK\n");
    a = 85; m = 9223372036854775808u;
    ai = umul_inv(a, m);
    printf("a = %d, m = %"PRIu64", a^-1 mod m = %"PRIu64"", a, m, ai);
    if (ai != 9006351518340545789u) {
        printf(" <- inversion error\n");
        exit(1);
    }
    else
        printf(" OK\n");

// -- for test umul_inv --
/*printf("for test umul_inv\n"); 
    a = 1;
    m = 9223372036854775808;
    while(a<256)
    {
	    ai = umul_inv(a,m);
	    if(ai == 0)	printf("a = %d, m = %d, a^-1 mod m = none\n",a,m);
	    else	printf("a = %d, m = %d, a^-1 mod m = %"PRIu64"\n",a,m,ai);
	    a++;
    }
  
    for(m = 2; m < 256; ++m)
	    for(a = 2; a < 256; ++a){
		    ai = umul_inv(a,m);
		    if(ai == 0) continue;
		    if((a*ai)%m != 1){
			    printf("error\n");
			    exit(1);
		    }
	    }

   printf("not error\n");
*/  
printf("Congratulations!\n");
    return 0;
}

