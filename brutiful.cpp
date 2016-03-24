/** @file
 * Brute force program. Iterates through all possible values of a buffer given
 * a particular set of characters and runs each string value through an
 * evaluator function to determine whether it was correct. First used for
 * BetaFour challenge on BCTF 2016, linking libcrypto and using SHA256
 * functions. Worked pretty well.
 *
 * Sample output:
 * 	Trying 
 * 	Trying !
 * 	Trying "
 * 	Trying #
 * 	Trying $
 * 	...
 * 	Trying H`|
 * 	Trying H`}
 * 	Trying H`~
 * 	Trying H`
 * 	Trying Ha!
 * 	Solution: Ha!
 */

#include <stdio.h>
#include <exception>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/** ASCII printable start. TODO: Adjust to fit your needs. */
#define CHAR_FIRST	33
/** ASCII printable end. TODO: Adjust to fit your needs. */
#define CHAR_LAST	126

/** Debug enable/disable. Mainly determines whether the stock evaluator
 * implemented below will print the content of each buffer it is evaluating.
 * Naturally, you want to disable this when you're done testing, because the
 * I/O dramatically worsens performance. */
#define DEBUG 1

#if DEBUG
#define PDEBUG printf
#else
#define PDEBUG
#endif

/** Solution evaluator callback type . Indicates to the @iterate function
 * whether the current value of the buffer satisfies the problem criteria. You
 * must implement one of these to make this program useful.
 * @param Current value of the buffer to evaluate */
typedef bool (*evalfunc)(unsigned char *val);

/** Incrementing string. Iterates through all values for that string given the
 * specified character set. The string is represented in memory as a vector of
 * digits composing a number having a radix equal to the number of characters
 * in the chosen character set. The value of each "digit" is used to index into
 * the character set when the string's value must be retrieved. The character
 * set is represented explicitly so that character sets can later be introduced
 * that would comprise multiple disjoint ranges of ASCII values. */
class IncString {
	public:
		/** IncString constructor. Allocates a pre-sized buffer in which to
		 * increment the string from all firstchar to all lastchar. Also
		 * allocates and initializes a pre-sized buffer to index against when
		 * returning the string's current value
		 * @param len the length of the buffer to create and iterate
		 * @param firstchar the first ASCII value to include in the character
		 * 	set
		 * @param lastchar the last ASCII value to include in the character
		 *  set */
		IncString(size_t len, size_t firstchar, size_t lastchar) {
			_len = len;
			_charset_len = lastchar - firstchar + 1;

			_buf = new uint8_t [len];
			if (_buf == NULL) { throw std::exception(); }

			/* The array is initialized to zero instead of firstchar because
			 * when the value of the string is taken, this number (0) will
			 * correspond to _charset[0], whereas _charset[firstchar] would be
			 * undefined (firstchar may be greater than the number of
			 * characters in the charset). */
			memset(_buf, 0, len);

			_charset = new unsigned char [_charset_len];
			if (_charset == NULL) { throw std::exception(); }

			for (size_t i=0, c=firstchar; i<_charset_len; i++, c++) {
				_charset[i] = c;
			}
		}

		/** IncString destructor. Deallocates buffers. */
		~IncString() {
			if (_buf) delete [] _buf;
			if (_charset) delete [] _charset;
		}

		/** Get string value. Writes the current value of the string to an
		 * array
		 * @param array the array to write to */
		void Value(unsigned char *array) {
			for (size_t i=0; i<_len; i++) {
				array[i] = _charset[_buf[i]];
			}
		}

        /** Increment string. Increments the least significant element of the
         * string, rolling over and moving to the next digit if necessary. */
		bool Increment() {
			bool ret = false;
			for (signed int i=(_len-1); i>=0; i--) {
				if (_buf[i] == _charset_len) {
					_buf[i] = 0;
					ret = (i==0);
				} else {
					_buf[i]++;
					break;
				}
			}

			return ret;
		}

		/** Display character set. Shows all the characters in the character
		 * set. */
		void DumpCharset() {
			printf("_charset_len = %ld\n", (unsigned long)_charset_len);
			for (size_t i=0; i<_charset_len; i++) {
				printf("%c", _charset[i]);
			}
			printf("\n");
		}

	protected:
		size_t _len;
		size_t _charset_len;
		uint8_t *_buf;
		unsigned char *_charset;
};

int Usage(FILE *out, char *progname, int ret);
bool iterate(unsigned char *trybuf, size_t len, size_t start, evalfunc eval);
bool try_a_value(unsigned char *val);

/** Entry point. Requires a buffer length argument and accepts an optional
 * string argument to initialize the buffer. */
int
main(int argc, char **argv) {
	unsigned char *startswith = NULL;
	size_t strlen_startswith = 0;
	unsigned char *trybuf = NULL;
	size_t len = 0;

	if ((argc > 3) || (argc == 1)) {
		return Usage(stderr, argv[0], 1);
	}

	len = atoi(argv[1]);
	if (len <= 0) {
		fprintf(stderr, "Invalid len");
		return Usage(stderr, argv[0], 1);
	}

	trybuf = (unsigned char *)malloc(len+1);
	if (trybuf == NULL) {
		fprintf(stderr, "malloc failed\n");
		return 1;
	}

	if (argc == 3) {
		startswith = (unsigned char *)argv[2];
		strlen_startswith = strlen((const char *)startswith);
	}

	memset(trybuf, 0, len+1);

	if (startswith) {
		strncpy(
			(char *)trybuf,
			(const char *)startswith,
			(strlen_startswith <= len)? strlen_startswith: len);
	}

	if (iterate(trybuf, len, strlen_startswith, try_a_value)) {
		printf("Solution: %s\n", trybuf);
	} else {
		printf("WTF\n");
	}

	return 0;
}

/** Iterates through brute force. Checks the starting string first, then add
 * one character, then two, until you have tried all combinations of strings
 * starting with the starting string and having <= len characters given the
 * character set specified in the C preprocessor macros defined above.
 *
 * @param trybuf - Buffer in which to iterate
 * @param len - Number of valid bytes within trybuf
 * @param start - Byte offset in trybuf at which modification can begin
 * @param eval - Evaluator function for determining when we have succeeded */
bool
iterate(unsigned char *trybuf, size_t len, size_t start, evalfunc eval) {
	/* TODO: Include length so that eval knows what to do with the buffer */
	if (eval(trybuf)) { return true; }

	for (size_t i=(start+1); i<=len; i++) {
		IncString s(i-start, CHAR_FIRST, CHAR_LAST);
		do {
			s.Value(trybuf+start);
			if (eval(trybuf, i)) { return true; }
		} while (!s.Increment());
	}

	return false;
}

/** Sample evaluator callback. TODO: Reimplement this to fit your needs.
 * @param val the current value of the buffer that should be evaluated. */
bool
try_a_value(unsigned char *val)
{
	PDEBUG("Trying %s\n", val);
	return (!strcmp((const char *)val, "Ha!"));
}

/** Usage. Explains command-line arguments, outputs to either stdout or stderr
 * as supplied, and returns errorlevel for main to use as a one-liner exit.
 * @param out a FILE stream to output to, expecting either stdout or stderr
 * @param progname the value of argv[0]
 * @param ret the return value to return
 */
int
Usage(FILE *out, char *progname, int ret) {
	fprintf(out, "Usage: %s buflen [startswith]\n", progname);
	return ret;
}
