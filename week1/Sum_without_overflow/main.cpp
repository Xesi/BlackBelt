#include <iostream>
#include <algorithm>
#include <optional>
#include <numeric>
#include <limits.h>

using namespace std;

typedef int64_t ll;

optional<ll> sum(ll a, ll b) {
	bool da = (a >= 0);
	bool db = (b >= 0);
	bool is_same_neg = (da && db) || (!da && !db);

	if (da && db && b > INT64_MAX - a) {
		return nullopt;
	}

	if (!da && !db && a < INT64_MIN - b) {
		return nullopt;
	}

	else return a + b;
}

int main() {
	ll a, b;
	cin >> a >> b;
	auto result = sum(a, b);
	if (!result.has_value()) cout << "Overflow!";
	else cout << *result;
}