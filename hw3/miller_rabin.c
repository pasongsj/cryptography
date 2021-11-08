/*
 * Copyright 2020, 2021. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로 제작되었습니다.
 */
#include "miller_rabin.h"

/*
 * Miller-Rabin Primality Testing against small sets of bases
 *
 * if n < 2^64,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
 *
 * if n < 3,317,044,064,679,887,385,961,981,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, and 41.
 */
const uint64_t a[ALEN] = {2,3,5,7,11,13,17,19,23,29,31,37};

/*
 * miller_rabin() - Miller-Rabin Primality Test (deterministic version)
 *
 * n > 3, an odd integer to be tested for primality
 * It returns 1 if n is prime, 0 otherwise.
 */
int miller_rabin(uint64_t n)
{
	uint64_t k,q,index,is_prime;
	q = n-1;
	k = 0;
	while(q%2==0){//q값이 홀수가 될때까지 나누어줌
		q = q/2;
		k++;
	}
	
	uint64_t bin_j;
	index = 0;
	if(n == a[0] || n == a[1])	return PRIME; // 2, 3인경우 while문에 들어가지 않음
	while(a[index] < n-1 && index<12){
		is_prime = COMPOSITE; //is_prime초기화
		if(mod_pow(a[index],q,n) == 1)	is_prime = PRIME;
		else{			
			for(uint64_t j = 0; j < k; j++)
			{
				if(j == 0)	bin_j = mod_pow(a[index],q,n);//초기값	
				else	bin_j = mod_mul(bin_j,bin_j,n);//누적값 연산
				
				if(bin_j == n-1)	is_prime = PRIME;
			}
		}
		if(is_prime == COMPOSITE)	return is_prime; 
		index++;
	}
	return is_prime;//모든 a값에 대해 PRIME으로 연산되는 경우만 PRIME이라고 확신할 수 있다.		

// 여기를 완성하세요
}
