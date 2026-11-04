#pragma once

#include "utils.h"
#include <stdint.h>

typedef struct {
    uint32_t n; // modulus
    uint32_t e; // public exponent
} RSA_Public_Key;

typedef struct {
    RSA_Public_Key pubkey;
    uint32_t privkey;
} RSA_Key_Pair;

const uint32_t FERMAT_PRIMES[] = {65537, 257, 17, 5, 3}; // for e selection


static uint32_t mod_inverse(uint32_t a, uint32_t m) {
    int32_t old_r = m, r = a;
    int32_t old_s = 0, s = 1;
    while (r != 0) {
        int32_t quotient = old_r / r;
        int32_t temp_r = r;
        r = old_r - quotient * r;
        old_r = temp_r;

        int32_t temp_s = s;
        s = old_s - quotient * s;
        old_s = temp_s;
    }
    if (old_s < 0) {
        old_s += m;
    }
    return old_s;
}

static uint32_t mod_pow(uint32_t base, uint32_t exp, uint32_t mod) {
    uint32_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = ((uint64_t)result * base) % mod;
        }
        exp = exp >> 1;
        base = ((uint64_t)base * base) % mod;
    }
    return result;
}


static uint32_t pow(uint32_t base, uint32_t exp) {
    uint32_t result = 1;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = result * base;
        }
        exp = exp >> 1;
        base = base * base;
    }
    return result;
}

uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

uint32_t lcm(uint32_t a, uint32_t b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return (a / gcd(a, b)) * b;
}

uint32_t carm_prime(uint32_t p, uint32_t k) {
    if (p < 2 || k < 1) {
        return 0; // Invalid input
    }
    // for p=2 and k >= 3 carmichael value is 2^k-2
    if (p == 2) {
        return (k >= 3) ? 1UL << (k - 2) : 1UL << (k - 1);
    }
    // otherwise carmichael value for a prime power is p^(k-1)*(p-1)
    return pow(p, k - 1) * (p - 1);
}

uint32_t carmichael(uint32_t n) {
    if (n < 2) {
        return n;
    }
    uint32_t result = 1;
    for (uint32_t p = 2; p * p <= n; ++p) {
        // for each prime factor of n divide it out from n and calculate its carmichael value
        if (n % p == 0) {
            uint32_t k = 0;
            while (n % p == 0) {
                n /= p;
                k++;
            }
            result = lcm(result, carm_prime(p, k));
        }
    }
    // we can only have at most one prime factor greater than sqrt(n) so any leftover tmp is prime
    if (n > 1) {
        result = lcm(result, carm_prime(n, 1));
    }
    return result;
}

RSA_Key_Pair generate_rsa_key_pair(uint32_t p, uint32_t q) {
    RSA_Key_Pair key_pair;
    key_pair.pubkey.n = p * q;
    uint32_t lambda_n = carmichael(p * q);
    uint32_t e_ind = 0;
    uint32_t e = FERMAT_PRIMES[e_ind];
    while (e_ind < sizeof(FERMAT_PRIMES) / sizeof(FERMAT_PRIMES[0]) && (e >= lambda_n || gcd(e, lambda_n) != 1)) {
        e_ind++;
        e = FERMAT_PRIMES[e_ind];
    }
    
    ASSERT(e_ind < sizeof(FERMAT_PRIMES) / sizeof(FERMAT_PRIMES[0])); // no valid e found

    key_pair.pubkey.e = e;
    key_pair.privkey = mod_inverse(e, lambda_n);

    return key_pair;
}

