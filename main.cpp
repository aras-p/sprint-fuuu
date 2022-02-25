#include <chrono>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <charconv>
#include <sstream>

#define HAS_FORMAT (__cplusplus >= 202110L)
#if HAS_FORMAT
#include <format>
#endif


#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

using namespace std;

static int g_Global;
static const int kMaxThreads = 8;
static const int kIterations = 2000000;
static const size_t kExpect = 114888893;

static void do_snprintf(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		snprintf(buf, 100, "%i", i + g_Global);
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of snprintf %i!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_snprintf_l(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		snprintf_l(buf, 100, NULL, "%i", i + g_Global);
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of snprintf_l %i!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_stb_snprintf(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		stbsp_snprintf(buf, 100, "%i", i + g_Global);
		sum += strlen(buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of stbsp_snprintf %i!\n", kExpect, sum, index);
		exit(1);
	}
}

static void do_stringstream(int index)
{
	size_t sum = 0;
	stringstream buf;
	for (int i = 0; i < kIterations; ++i) {
		buf.seekp(0);
		buf << (i + g_Global);
		int len = (int)buf.tellp();
		buf.seekg(0);
		int firstchar = buf.get();
		sum += len + firstchar;
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of stringstream %i!\n", kExpect, sum, index);
		exit(1);
	}
}

#if HAS_FORMAT
static void do_format_to(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		const auto res = format_to_n(buf, 100, "{}", i + g_Global);
		sum += (res - buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread of format_to_n %i!\n", kExpect, sum, index);
		exit(1);
	}
}
#endif


static void do_to_chars(int index)
{
	size_t sum = 0;
	char buf[100];
	for (int i = 0; i < kIterations; ++i) {
		to_chars_result res = to_chars(buf, buf+100, i + g_Global);
		sum += (res.ptr - buf) + buf[0];
	}
	if (sum != kExpect) {
		printf("    sum was supposed to be %zi, but got %zi in thread %i of to_chars!\n", kExpect, sum, index);
		exit(1);
	}
}

typedef void (*thread_func)(int index);

static void do_test_with_func(const char* name, thread_func func)
{
	printf("==== Test %s:\n", name);
	vector<float> times;
	for (int i = 1; i <= kMaxThreads; ++i) {
		printf("%i threads...\n", i);
		vector<thread> threads;
		auto t0 = chrono::steady_clock::now();
		for (int j = 0; j < i; ++j)
			threads.emplace_back(thread(func, j));
		for (auto &t : threads)
			t.join();
		auto t1 = chrono::steady_clock::now();
		chrono::duration<float, milli> ms = t1 - t0;
		float msf = ms.count();
		printf("  took %.1fms\n", msf);
		times.push_back(msf);
	}
	printf("Threads;TimeMS\n");
	for (int i = 0; i < times.size(); ++i)
	{
		printf("%i;%.1f\n", i+1, times[i]);
	}
}

int main(int argc, const char** argv)
{
	g_Global = argc;
	do_test_with_func("snprintf", do_snprintf);
	do_test_with_func("snprintf_l", do_snprintf_l);
	do_test_with_func("stb_snprintf", do_stb_snprintf);
	do_test_with_func("stringstream", do_stringstream);
#if HAS_FORMAT
	do_test_with_func("format_to_n", do_format_to);
#endif
	do_test_with_func("to_chars", do_to_chars);
	return 0;
}
