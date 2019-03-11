#include <stdlib.h>
#include <assert.h>
#include "cht.h"

//@ #include "modulo.gh"

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/*@

	predicate prime(int x) = prime_div(x, 2);
	
	predicate prime_div(int x, int div) =
		div >= x ? //TODO: not the best bound (sqrt(x))
			true
		:
			false == ((x/div)*div != x) &*&
			prime_div(x, div + 1);
	
	predicate coprime(int x, int y) = coprime_div(x, y, 2);
	
	predicate coprime_div(int x, int y, int div) =
		div >= x ? //TODO: not the best bound (min(sqrt(x), sqrt(y)))
			true
		:
			false == ( ((x/div)*div != x) && ((y/div)*div != y) ) &*&
			coprime_div(x, y, div + 1);
	
	predicate sub_permut(list<int> xs, int max_val) =
		xs == nil ?
			true
		:
			true == forall(xs, (lt)(max_val)) &*&
			true == forall(xs, (ge)(0)) &*&
			true == no_dups(xs);
				
				
	fixpoint list<t> sub_list<t>(list<t> xs, int size) {
		switch(xs) {
			case nil: return nil;
			case cons(x0, xs0): return (size <= 0 ? nil : cons(x0, sub_list(xs0, size - 1)) );
		}
	}
	
	lemma void sub_list_zerosize<t>(list<t> xs)
		requires true;
		ensures sub_list(xs, 0) == nil;
	{
		switch(xs) {
			case nil:
				assert (sub_list(xs, 0) == nil);
			case cons(x0, xs0):
				assert (sub_list(xs, 0) == nil);
		}
	}
				
	lemma void less_than_modulo(int k, int m)
		requires 0 <= k &*& k < m &*& 0 < m;
		ensures k % m == k;
	{
		division_round_to_zero(k, m);
		div_rem_nonneg(k, m);
	}
	
	/*lemma void modulo_mul_coprime(int k, int s, int m)
		requires
			0 < k &*& k < m &*&
			0 < s &*& s < m &*&
			0 < m &*& 
			(k % m != 0) &*& coprime(s, m);
		ensures
			(k * s) % m != 0 &*& coprime(s, m);
	{
		div_rem_nonneg(k, m);
		assert (k != k / m * m);
	}*/
	
	lemma void mul_diff(int a, int b, int s) 
		requires a != b &*& s > 0;
		ensures s * a != s * b;
	{
		assume (s * a != s * b);//TODO
	}
	
	lemma void mul_nonzero(int a, int b)
		requires a > 0 &*& b > 0;
		ensures a * b > 0;
	{
		mul_nonnegative(a - 1, b);
	}
	
	lemma void modulo_permutation(int a, int b, int s, int m) 
		requires 
			0 <= a &*& a < m &*&
			0 <= b &*& b < m &*&
			0 < s &*& s < m &*&
			0 < m &*& 
			b > a &*& coprime(s, m);
		ensures
			((s * a) % m) != ((s * b) % m) &*& coprime(s, m) ;
	{
		less_than_modulo(b - a, m);
		div_rem_nonneg(b - a, m);
		mul_nonnegative(s, b - a);
		
		assert ((b - a) != (b - a) / m * m);
		
		mul_diff((b - a), (b - a) / m * m, s);
		mul_nonzero(s, b - a);
				
		div_rem_nonneg(s * (b - a), m);
		assert (s * (b - a) != ( (s * (b - a)) / m) * m);
		assert ((s * (b - a)) % m != 0 );
	} 

@*/


static
uint64_t loop(uint64_t k, uint64_t capacity)
//@ requires 0 < capacity &*& capacity < INT_MAX;
//@ ensures 0 <= result &*& result < capacity &*& result == k%capacity;
{
  uint64_t g = k%capacity;
  //@ div_mod_gt_0(g, k, capacity);
  return g;
}

int main()
	//@ requires true;
	//@ ensures true;
{
	uint32_t cht_height = 7;
	uint32_t backend_capacity = 3; 
	uint32_t i = 0;

	int* permutations = malloc(sizeof(int)*(int)(cht_height));
	if (permutations == 0) abort();

	uint32_t offset_absolut = i*31;
	uint64_t offset = loop(offset_absolut, cht_height);
	uint64_t base_shift = loop(i, cht_height - 1);
	uint64_t shift = base_shift + 1;
	//@ open ints(permutations, cht_height, ?p0);
	//@ close ints(permutations, cht_height, p0);
	//@ close sub_permut(nil, cht_height);
	for (uint32_t j = 0; j < cht_height; ++j)
	/*@ invariant 
		0 <= j &*& j <= cht_height &*&
		ints(permutations, cht_height, ?p) &*&
		sub_permut(sub_list(p, j), cht_height);
	@*/
	{		
		uint64_t permut = loop(offset + shift*j, cht_height);
		// Prove that permut is different that any other "previous" permut 
		
		permutations[j] = (int)permut;
	}
	free(permutations);
	return 0;
}