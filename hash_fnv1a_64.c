/*
 * Copyright 2026 Molino Rosario Walter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * --- IMPORTANT NOTICE ---
 * THIS SOFTWARE IS PROVIDED SOLELY FOR EDUCATIONAL AND ACADEMIC PURPOSES.
 * IT IS NOT INTENDED AND MUST NOT BE USED IN PRODUCTION ENVIRONMENTS.
 */


#include <stdio.h>
#include <stdint.h>

/*
hash = offset_basis
for each octet_of_data to be hashed
        hash = hash xor octet_of_data
        hash = hash * FNV_prime
return hash

http://www.isthe.com/chongo/tech/comp/fnv/

*/

uint64_t hashing_fnv1a(const char * str,int size){
    const uint64_t offset_basis = 14695981039346656037ULL;
    const uint64_t FNV_prime = 1099511628211ULL;
    uint64_t hash = offset_basis;

    while(*str){
        hash = hash ^ (uint64_t) (unsigned char) (*str);
        hash = hash * FNV_prime;

        ++str;
    }
    return hash % size;

}

/*
int main(){
    const char *c = "ciaoreredef";
    uint64_t myhash=  hashing_fnv1a(c,10);
    printf("%llu\n", (unsigned long long)myhash);
    printf("\n");
    const char *c2 = "ciao";
    uint64_t myhash2=  hashing_fnv1a(c2,10);
    printf("%llu\n", (unsigned long long)myhash2);

    return 0;
}

*/