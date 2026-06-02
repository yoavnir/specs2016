#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include "utils/platform.h"
#include "cli/tokens.h"
#include "specitems/specItems.h"
#include "processing/Config.h"
#include "processing/ProcessingState.h"
#include "processing/StringBuilder.h"
#include "processing/Reader.h"

extern ALUCounters g_counters;
extern char        g_printonly_rule;
extern bool        g_keep_suppressed_record;
extern unsigned int g_WhileGuardLimit;

//  DUMP_TO_FILES levels:
//   0 - Never dump to files
//   1 - Dump to files only for failed tests (anything but "Res")
//   2 - Always dump to files - later tests will overwrite previous tests

#define DUMP_TO_FILES 1

std::string prettify(std::string label, std::string src)
{
#ifdef DUMP_TO_FILES
#if DUMP_TO_FILES > 0
#if DUMP_TO_FILES == 1
    if ("Res" != label) {
#endif
	std::string fname = std::string("ProcessingTest.")+label;
	auto f = fopen(fname.c_str(), "w");
	fprintf(f, "%s", src.c_str());
	fclose (f);
#if DUMP_TO_FILES == 1
    }
#endif
#endif
#endif
	std::string ret(label);
	int max_chars = (label == "Res") ? 160 : 256;
	ret += ": <";
	for (char c : src) {
		if (max_chars <= 0) {
			if (max_chars == 0) {
				ret += "...";
				max_chars--;
			}
			continue;
		}
		switch (c) {
			case '\n':
				ret.append("\\n");
				break;
			case '\t':
			    do {
					ret+=' ';
				} while (0 != ret.size() % 4);
				break;
			default:
				ret+=c;
		}
		max_chars--;
	}
	ret += ">";
	return ret;
}

#define VERIFY(sp,ex) do {          \
		testCount++;                            \
		if (onlyTest!=0 && onlyTest != testCount) break;  \
		PSpecString ps = runTestOnExample(sp, "The quick brown fox jumped over the   lazy dog");  \
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testCount << " ";     \
		if (!ps) {                              \
			std::cout << "*** NOT OK ***\n\tGot: (NULL)\n\tExpected: <" << ex << ">\n"; \
			errorCount++;                       \
			failedTests.push_back(testCount);      \
		} else {                                \
			if (*(ps) != std::string(ex)) {     \
				std::cout << "*** NOT OK ***\n\t" << prettify("Got", *ps) << "\n\t" << prettify("Expected", ex) << "\n"; \
				errorCount++;                   \
				failedTests.push_back(testCount);  \
			} else {                            \
				std::cout << "***** OK *****   " << prettify("Res", ex) << "\n"; \
			}                                   \
		}                                       \
} while (0);

#define VERIFY2(sp,ln,ex) do {          \
		testCount++;                            \
		if (onlyTest!=0 && onlyTest != testCount) break;  \
		PSpecString ps = runTestOnExample(sp, ln);  \
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testCount << " ";     \
		if (!ps) {                              \
			std::cout << "*** NOT OK ***\n\tGot: (NULL)\n\t" << prettify("Expected", ex) << "\n"; \
			errorCount++;                       \
			failedTests.push_back(testCount);      \
		} else {                                \
			if (*(ps) != std::string(ex)) {              \
				std::cout << "*** NOT OK ***\n\t" << prettify("Got", *ps) << "\n\t" << prettify("Expected", ex) << "\n"; \
				errorCount++;                   \
				failedTests.push_back(testCount);  \
			} else {                            \
				std::cout << "***** OK *****   " << prettify("Res", ex) << "\n"; \
			}                                   \
		}                                       \
} while (0);

#define VERIFYCMD(cmd,res) do {                 \
	testCount++;                                \
	if (onlyTest!=0 && onlyTest != testCount) break;  \
	std::string actual_res("");                 \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testCount << " ";     \
	try { cmd; }                                \
	catch(SpecsException& e) {                  \
		actual_res = e.what(true);              \
	}                                           \
	if (res==actual_res) {                      \
		std::cout << "***** OK *****   " << prettify("Res", actual_res) << "\n"; \
	} else {                                    \
		errorCount++;                           \
		failedTests.push_back(testCount);          \
		std::cout << "*** NOT OK ***\n\t" << prettify("Got", actual_res) << "\n\t" << prettify("Expected", res) << ">\n"; \
	}                                           \
} while (0);


PSpecString runTestOnExample(const char* _specList, const char* _example)
{
	ProcessingState ps;
	ProcessingStateFieldIdentifierGetter fiGetter(&ps);
	setFieldIdentifierGetter(&fiGetter);
	setStateQueryAgent(&ps);
	classifyingTimer tmr;
	PWriter pwr0 = std::make_shared<StringWriter>();
	PStringWriter pwr1 = std::make_shared<StringWriter>();

	PWriter writerArray[] = {pwr0,pwr1};
	ps.setWriters(writerArray);

	g_counters.clearAll();
	g_keep_suppressed_record = false;
	g_printonly_rule = PRINTONLY_PRINTALL;
	g_forwardContext = 0;
	g_backwardContext = 0;

	TestReader tRead(100);
	g_pReader = &tRead;
	unsigned int readerCounter = 1;
	char* example = strdup(_example);
	char* example_ctx = example;
	char* ln = strtok_r(example, "\n", &example_ctx);
	while (ln) {
		tRead.InsertString(ln);
		ln = strtok_r(nullptr, "\n", &example_ctx);
	}

	char* specList = (char*)_specList;

	itemGroup ig;

	PSpecString result = nullptr;
	StringBuilder sb;
	setPositionGetter(&sb);

	unsigned int index = 0;

	std::vector<Token> vec = parseTokens(1, &specList);

	try {
		normalizeTokenList(&vec);
	} catch (const SpecsException& e) {
		result = std::make_shared<std::string>(e.what(true));
		goto end;
	}

	try {
		ig.Compile(vec,index);
	} catch (const SpecsException& e) {
		result = std::make_shared<std::string>(e.what(true));
		goto end;
	}

	try {
		if (ig.readsLines() || !ig.needRunoutCycle()) {
			do {
				PSpecString pFirstLine = tRead.getNextRecord();
				ps.setString(pFirstLine);
				ps.setFirst();
				ps.incrementCycleCounter();
				bool bSomethingWasDone = ig.processDo(sb, ps, &tRead, tmr, readerCounter);
				if (ps.printSuppressed(g_printonly_rule) && g_keep_suppressed_record) {
					continue;
				}
				PSpecString pWritten = pwr1->getString();
				PSpecString pOut = sb.GetStringUnsafe();
				if (!pOut && bSomethingWasDone) {
					pOut = std::make_shared<std::string>();
				}
				if (ps.shouldWrite() && !ps.printSuppressed(g_printonly_rule)) {
					while (pWritten) {
						if (result) *result = *result + '\n' + *pWritten;
						else result = pWritten;
						pWritten = pwr1->getString();
					}
					if (result) {
						if (pOut) *result = *result + '\n' + *pOut;
					}
					else result = pOut;
				} else {
				}
			} while (!tRead.endOfSource());
		}
	} catch (SpecsException& e) {
		if (result) {
		}
		result = std::make_shared<std::string>(e.what(true));
		goto end;
	}

	if (ig.needRunoutCycle()) {
		if (!ig.readsLines()) {
			ig.setRegularRunAtEOF();
		}
		ps.setEOF();
		ps.setString(nullptr);
		ps.setFirst();
		try {
			ig.processDo(sb, ps, nullptr, tmr, readerCounter);
			PSpecString pWritten = pwr1->getString();
			PSpecString pOut = sb.GetStringUnsafe();
			if (pWritten) {
				if (result) *result = *result + '\n' + *pWritten;
				else result = pWritten;
			}
			if (result) *result = *result + '\n' + *pOut;
			else result = pOut;
		} catch (SpecsException& e) {
			result = std::make_shared<std::string>(e.what(true));
			goto end;
		}
	}

end:
	g_pReader = nullptr;
	free(example);
	while (!vec.empty()) {
		vec[0].deallocDynamic();
		vec.erase(vec.begin());
	}
	return result ? result : std::make_shared<std::string>();
}

PSpecString runTestOnExample(std::string& s, const char* _example)
{
	return runTestOnExample(s.c_str(), _example);
}

int main(int argc, char** argv)
{
	g_bVerbose = true;
	int errorCount = 0;
	int testCount  = 0;
	int onlyTest   = 0;
	std::vector<int> failedTests;

	std::string spec;
	std::string strm;

	if (argc > 1) onlyTest = std::stoi(argv[1]);

	std::string key("version");
	std::string value("1.0");
	configSpecLiteralSet(key,value);

	specTimeSetTimeZone("UTC-2"); // All the time-format tests were set based on this time zone

	VERIFY("w1 1", "The"); // Test #1
	VERIFY("7-17 1", "ick brown f"); // Test #2
	VERIFY("2;-2 1", "he quick brown fox jumped over the   lazy do"); // Test #3
	VERIFY("20-* 1", " jumped over the   lazy dog"); // Test #4
	VERIFY("w3-5 1", "brown fox jumped"); // Test #5
	VERIFY("word 3-5 1", "brown fox jumped"); // Test #6
	VERIFY("word 8-9 1", "lazy dog"); // Test #7
	VERIFY("word 8-* 1", "lazy dog"); // Test #8
	VERIFY("w1 1 w3 nw w6 n", "The brownover"); // Test #9
	VERIFY("w1 2 w3 4 w7-8 12", " Thbrown   the   lazy"); // Test #10
	VERIFY("w1 1 w3 nf", "The\tbrown"); // Test #11
	VERIFY("substring 2-4 of word 3 1", "row"); // Test #12
	VERIFY("substring word 3;-2 of 1-* 1", "brown fox jumped over the   lazy"); // Test #13
	VERIFY("substring wordsep e w3 of 1-* 1", "d ov"); // Test #14
	VERIFY("substring fieldsep e w3 of 1-* 1", "brown"); // Test #15
	VERIFY("substring fieldsep / / field 4 of w7-* 1", "lazy"); // Test #16
	VERIFY("w1 1.5 w2 n.5 w3 n.4 w4 n","The  quickbrowfox"); // Test #17
	VERIFY("w1 1.8 right w2 n.8 center w3 n left", "     The quick  brown"); // Test #18
	VERIFY("33.6 strip 1.6 right", "   the"); // Test #19
	VERIFY("w1 C2X 1 w7 C2B nw /30313233/ X2CH nw","546865 011101000110100001100101 0123"); // Test #20
	VERIFY2("fs = field 1 1 field 2 6 field 3 11 field 4 16", "a=b", "a    b         "); // Test #21
	VERIFY2("fs = field 1 1 field 2 6 field 3 11 field 4 16", "=a", "     a         ");  // Test #22
	VERIFY2("fs = field 1 1 field 2 6 field 3 11 field 4 16", "==a=b", "          a    b"); // Test #23
	VERIFY("word -2 1", "lazy"); // Test #24
	VERIFY("word 2;-2 1", "quick brown fox jumped over the   lazy"); // Test #25
	VERIFY("word 2.3 1", "quick brown fox"); // Test #26
	VERIFY2("substring fieldsep . field 1 of substr word 1 of fs = f 1   1", "  a = 17", "a"); // Test #27
	VERIFY2("substring fieldsep . field 1 of substr word 1 of fs = f 1   1", "x.18= 23", "x"); // Test #28
	VERIFY("number 1 w2 nw number strip nw", "         1 quick 1"); // Test #29
	VERIFY2("wordseparator e w2 1 w4 nw", "Hope is the thing with feathers", " is th ath"); // Test #30
	VERIFY2("w1 1 read w1 10", "Once there were green fields\nKissed by the sun","Once     Kissed"); // Test #31
	VERIFY("pad /q/ w1 1.10 left pad /w/ w2 11.10 center pad /e/ w3 21.10 right","Theqqqqqqqwwquickwwweeeeebrown"); // Test #32
	VERIFY("2:3 1 -7:-5 nw -2:* nw", "he azy og"); // Test #33
	VERIFY("k: w2 . ID k 1", "quick"); // Test #34
	VERIFY2("1-* tf2i %Y-%m-%dT%H:%M:%S.%6f a: ID a ti2f /%A, %B %drd, %Y; %M minutes past the %Hth hour/ 1", "2018-11-23T14:43:43.126573","Friday, November 23rd, 2018; 43 minutes past the 14th hour"); // Test #35

	// Following issue #12
	VERIFY("w1 n", "The"); // Test #36
	VERIFY("w2 nw", "quick"); // Test #37 -- nextword starts at column 1 if string is empty
	VERIFY("w3 nf", "\tbrown"); // Test #38
	VERIFY("w5-* 1 w4 7 w1 nw", "jumpedfox Thehe   lazy dog"); // Test #39

	VERIFY("/100/ d2x 1", "64");                       // Test #40
	VERIFY("/100/ x2d 1", "256");                      // Test #41
	VERIFY("/10000000000/ d2x 1", "2540be400");        // Test #42
	VERIFY("/10000000000/ x2d 1", "1099511627776");    // Test #43
	VERIFY("/hello/ d2x 1", "Cannot convert <hello> from format <Decimal> to format <Hex>"); // Test #44
	VERIFY("/-5/ d2x 1", "fffffffffffffffb");          // Test #45
	VERIFY("/-5/ x2d 1", "18446744073709551611");      // Test #46

	VERIFY("/Star Trek/ ucase 1", "STAR TREK");        // Test #47
	VERIFY("/Star Trek/ lcase 1", "star trek");        // Test #48

#ifdef WIN64
	VERIFY("a: /1545407296548900/ . print 'tobin(a)' ti2f '%c' 1", "12/21/18 17:48:16");  // Test #49
	VERIFY("a: /1545407296548900/ . print 'tobin(a+3600000000)' ti2f '%c' 1", "12/21/18 18:48:16");  // Test #50
#else
	VERIFY("a: /1545407296548900/ . print 'tobin(a)' ti2f '%c' 1", "Fri Dec 21 17:48:16 2018");  // Test #49
	VERIFY("a: /1545407296548900/ . print 'tobin(a+3600000000)' ti2f '%c' 1", "Fri Dec 21 18:48:16 2018");  // Test #50
#endif

	// Issue #22
	VERIFY2("fs : field 1-* 1", "a:b", "a:b");  // Test #51

	// if...then...else...endif
	spec = "a: w1 . if 0=a%2 then even 1 else odd 1 endif";
	VERIFY2(spec, "2",     "even");    // Test #52
	VERIFY2(spec, "hello", "even");    // Test #53
	VERIFY2(spec, "7",     "odd");     // Test #54
	VERIFY2(spec, "7.5",   "odd");     // Test #55
	VERIFY2(spec, "8.5",   "even");    // Test #56

	spec = "  a: w1 .           " \
		   "  if a=1 then       " \
		   "      one 1         " \
		   "  elseif a=2 then   " \
		   "      two 1         " \
		   "  elseif a=3 then   " \
		   "      three 1       "\
		   "  else              " \
		   "      many 1        "\
		   "  endif             ";
	VERIFY2(spec, "1", "one");    // Test #57
	VERIFY2(spec, "2", "two");    // Test #58
	VERIFY2(spec, "2.0", "two");  // Test #59
	VERIFY2(spec, "3", "three");  // Test #60
	VERIFY2(spec, "4", "many");   // Test #61
	VERIFY2(spec, "5", "many");   // Test #62
	VERIFY2(spec, "yes", "many"); // Test #63

	spec = "  a: w1 .                       " \
		   "  if a>1 then                   " \
		   "      if a>3 then               " \
		   "          if a>5 then           " \
		   "              /very big/ 1      " \
		   "          else                  " \
		   "              /not too big/ 1   " \
		   "          endif                 " \
		   "      else                      " \
		   "          /quite small/ 1       " \
		   "      endif                     " \
		   "  else                          " \
		   "      /really small/ 1          " \
		   "  endif                         ";

	VERIFY2(spec, "0", "really small");   // Test #64
	VERIFY2(spec, "1", "really small");   // Test #65
	VERIFY2(spec, "2.5", "quite small");  // Test #66
	VERIFY2(spec, "4.5", "not too big");  // Test #67
	VERIFY2(spec, "6", "very big");       // Test #68

	// while...do...done
	spec =  "  a: w1 .         " \
			"  set #0:=a       " \
			"  while #0>0 do   " \
			"      print #0 n  " \
			"      set #0-=1   " \
			"  done            ";
	VERIFY2(spec, "5", "54321");      // Test #69
	VERIFY2(spec, "1", "1");          // Test #70
	VERIFY2(spec, "0", "");           // Test #71
	VERIFY2(spec, "-5", "");          // Test #72
	VERIFY2(spec, "3.14", "3.142.141.140.14");  // Test #73
	VERIFY2(spec, "yes", "");         // Test #74

	spec =  "  a: w1 1 /:/ n   " \
			"  set #0:=a       " \
			"  set #1:=2       " \
			"  while #0>1 do   " \
			"      if '0=#0 % #1' then  " \
			"          print #1 nw /(/ n  " \
			"          set #2:=0         " \
			"          while '(#0>1) & (0=#0 % #1)' do  " \
			"              set '#2 += 1'     " \
			"              set '#0 /= #1'   " \
			"          done                  " \
			"          print #2 n /)/ n      " \
			"      endif                     " \
			"      set '#1 += 1'             " \
			"  done                          ";
	VERIFY2(spec, "28", "28: 2(2) 7(1)");   // Test #75
	VERIFY2(spec, "1024", "1024: 2(10)");   // Test #76
	VERIFY2(spec, "67", "67: 67(1)");       // Test #77
	VERIFY2(spec, "0", "0:");               // Test #78
	VERIFY2(spec, "-3", "-3:");             // Test #79
	VERIFY2(spec, "to keep the number", "to:");// VERIFY2(spec, "6.2", "6.2: 2(1) 3(1)"); -- this goes into an endless loop
	VERIFY2(spec, "hello", "hello:");       // Test #81

	// Test run-out cycle
	VERIFY2("one 1 EOF two 2", " ", "one\n two");     // Test #82
	VERIFY2("two 2 EOF one 1", " ", " two\none");     // Test #83

	spec =  " a: w1 1.4 right                  " \
			"    set #0:=a*a                   " \
			"    set #3+=#0                    " \
			"    print #0 6.6 right            " \
			" EOF                              " \
			"    /Total:/ 1 print #3 nw        ";
	VERIFY2(spec, "1", "   1      1\nTotal: 1");                  // Test #84
	VERIFY2(spec, "3\n4", "   3      9\n   4     16\nTotal: 25"); // Test #85

	VERIFY2("a: w1 . if 'a==1' then w1 1 endif", "1", "1");       // Test #86
	VERIFY2("a: w1 . if 'a==1' then w1 1 endif", "0\n1\n2", "1"); // Test #87

	// issue #32 = word separator default and special
	VERIFY2("{ 1 w1 n } n",            "  \tword", "{word}");    // Test #88
	VERIFY2("ws / / { 1 w1 n } n",     "  \tword", "{\tword}");  // Test #89
	VERIFY2("ws default { 1 w1 n } n", "  \tword", "{word}");    // Test #90

	// tf2s and s2tf
	VERIFY2("1-* tf2s %Y-%m-%dT%H:%M:%S.%6f a: ID a s2tf /%A, %B %drd, %Y; %M minutes past the %Hth hour/ 1", "2018-11-23T14:43:43.126573","Friday, November 23rd, 2018; 43 minutes past the 14th hour"); // Test #91
#ifdef WIN64
	VERIFY("/1545407296.548900/ s2tf '%c' 1", "12/21/18 17:48:16");  // Test #92
	VERIFY("a: /1545407296.548900/ . print 'a+3600' s2tf '%c' 1", "12/21/18 18:48:16");  // Test #93
#else
	VERIFY("/1545407296.548900/ s2tf '%c' 1", "Fri Dec 21 17:48:16 2018");  // Test #92
	VERIFY("a: /1545407296.548900/ . print 'a+3600' s2tf '%c' 1", "Fri Dec 21 18:48:16 2018");  // Test #93
#endif

	// Issue #43
	VERIFY2("word 1 5 pad * word 2 15", "First record", "    First*****record"); // Test #94

	// Issue #44
	VERIFY("print \"'hello'\" 1", "hello"); // Test #95

	// NUMBER
	VERIFY2("w1 1 NUMBER nw", "One\nTwo\nThree", "One          1\nTwo          2\nThree          3"); // Test #96

	// recno and iterno
	VERIFY2("print 'recno()' 1 print 'number()' nw READ w1 nw", "a\nb\nc\nd","1 1 b\n3 2 d"); // Test #97

	// REDO
	VERIFY("w3-* 1 REDO w1 1", "brown");     // Test  #98
	VERIFY("6-* 1 REDO w1 1", "uick");       // Test  #99
	VERIFY("1-* BSWAP 1 REDO w2 1", "yzal"); // Test #100

	// SELECT SECOND
	spec =  "WORD 1        1 " \
			"SELECT SECOND   " \
			"WORD 1 NEXTWORD " \
			"SELECT FIRST    " \
			"WORD 2 NEXTWORD " \
			"SELECT SECOND   " \
			"WORD 2 NEXTWORD ";
	VERIFY2(spec, "first record\nsecond line\nlast one", "first record\nsecond first line record\nlast second one line\nlast one"); // Test #101

	// Statistics Pseudo-Functions
	spec =  "a: WORD 1 .                               " \
			" EOF                                      " \
			"   /AVG:/  1 PRINT 'average(a)'          N" \
			"   /STD:/ NW PRINT 'round(stddev(a),7)'  N" \
			"   /ERR:/ NW PRINT 'round(stderrmean(a),7)' N" \
			"   /VAR:/ NW PRINT 'variance(a)'         N" \
			"   /SUM:/ NW PRINT 'sum(a)'              N" \
			"   /MIN:/ NW PRINT 'min(a)'              N" \
			"   /MAX:/ NW PRINT 'max(a)'              N";
	VERIFY2(spec, "1\n2\n3\n4\n5", "AVG:3 STD:1.4142136 ERR:0.3535534 VAR:2 SUM:15 MIN:1 MAX:5"); // TEST #102

	spec = "a: WORD 1 ." \
		   " EOF " \
		   "   print 'fmap_nelem(a)'               1 " \
		   "   print 'fmap_nsamples(a)'           NW " \
		   "   print 'fmap_common(a)'             NW " \
		   "   print 'fmap_rare(a)'               NW " \
		   "   print 'fmap_count(a,3)'            NW " \
		   "   print 'round(fmap_frac(a,3),4)'    NW " \
		   "   print 'round(fmap_pct(a,3),3)'     NW /%/ N";
	VERIFY2(spec, "1\n2\n3\n4\n1\n5\n2\n3\n4\n3\n3", "5 11 3 5 4 0.3636 36.364%"); // TEST #103

	// random and statistics
	g_WhileGuardLimit = 10050;
	spec = "while '#0<10000' do                  " \
           "   print 'fmap_sample(a,rand(10))' . " \
           "   set '#0+=1'                       " \
           "done                                 " \
           "set '#1:=fmap_count(a,7)'            " \
           "if '#1 > 900 & #1 < 1100' then       " \
           "   /OK/ 1                            " \
           "else                                 " \
           "   /NOT OK/ 1                        " \
           "endif                                ";
	VERIFY(spec, "OK");  // TEST #104

	spec = "while '#0<10000' do                  " \
		   "   set '#2:=rand()'                  " \
		   "   if '#2 >= 0.7 & #2 < 0.8' then    " \
		   "      set '#1+=1'                    " \
		   "   endif                             " \
           "   set '#0+=1'                       " \
           "done                                 " \
           "if '#1 > 900 & #1 < 1100' then       " \
           "   /OK/ 1                            " \
           "else                                 " \
           "   /NOT OK/ 1                        " \
           "endif                                ";
	VERIFY(spec, "OK");  // TEST #105

	spec = "while '#0<10000' do                  " \
		   "   set '#2:=rand(10)'                " \
		   "   if '#2 = 3' then                  " \
		   "      set '#1+=1'                    " \
		   "   endif                             " \
           "   set '#0+=1'                       " \
           "done                                 " \
           "if '#1 > 900 & #1 < 1100' then       " \
           "   /OK/ 1                            " \
           "else                                 " \
           "   /NOT OK/ 1                        " \
           "endif                                ";
	VERIFY(spec, "OK");  // TEST #106

	spec = "word 1 1 noprint";
	VERIFY2(spec, "1 2\n2 3\n3 4\n2 2 2 2\n1\n\n", ""); // TEST #107

	// ASSERT and ABEND
	spec = "a: word 1 1 assert 'a<5'";
	VERIFY2(spec, "1\n2\n3\n4\n5\n6", "ASSERTION failed: a<5");  // TEST #108

	spec = "a: word 1 1 if 'a>4' then abend 'too big' endif";
	VERIFY2(spec, "1\n2\n3\n4\n5\n6", "ABEND: too big");  // TEST #109

	spec = "a: word 1 1 READ '+' N b: word 1 N '=' N print 'a+b' N";
	VERIFY2(spec, "1\n2\n3\n4\n5\n6", "1+2=3\n3+4=7\n5+6=11"); // TEST #110

	// locales
	VERIFYCMD(specTimeSetLocale("kuku",true),"Invalid locale <kuku>");  // TEST #111

#ifdef SPANISH_LOCALE_SUPPORTED
	VERIFYCMD(specTimeSetLocale("es_ES"),"");  // TEST #112
#ifdef PUT_TIME__SUPPORTED
	VERIFY("/1545407296.548900/ s2tf '%A,%d-%B-%Y' 1", "viernes,21-diciembre-2018");  // TEST #113
#else
	VERIFY("/1545407296.548900/ s2tf '%A,%d-%B-%Y' 1", "Friday,21-December-2018");  // TEST #113
#endif
#endif
	VERIFYCMD(specTimeSetLocale("C"),"");  // TEST #114
	VERIFY("/1545407296.548900/ s2tf '%A,%d-%B-%Y' 1", "Friday,21-December-2018");  // TEST #115
	
	VERIFY("print 'next()' 1 /next/ n print 'next()' n", "1next6");  // TEST #116

	VERIFY("/abcdefghijklmnopqrstuvwxyz/ (1,10,'R0')", "qrstuvwxyz"); // TEST #117
	VERIFY("/abcdefghijklmnopqrstuvwxyz/ (1,10,'L1')", "...tuvwxyz"); // TEST #118
	VERIFY("/abcdefghijklmnopqrstuvwxyz/ (1,10,'c2')", "ab...vwxyz"); // TEST #119
	VERIFY("/abcdefghijklmnopqrstuvwxyz/ (1,10,'R3')", "abc...wxyz"); // TEST #120
	VERIFY("/abcdefghijklmnopqrstuvwxyz/ (1,10,'l4')", "abcde...yz"); // TEST #121
	VERIFY("/abcdefghijklmnopqrstuvwxyz/ (1,10,'C5')", "abcdefg..."); // TEST #122
	VERIFY("/hello/ (1,10,'r3')", "     hello"); // TEST #123

	VERIFY("w1 1 w2 (,5) w3 n", "Thequickbrown") // TEST #124 - Issue #103

	// Issue #34
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' do /./ n set '#0-=1' done", "4...."); // TEST #125
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' /./ n set '#0-=1' done", "Missing DO after WHILE at index 6 with condition \"#0>0 .\""); // TEST #126
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' do /./ n set '#0-=1'", "4...."); // TEST #127 - meaningless after issue #145
	VERIFY("/4/ 1 done","DONE without WHILE at index 3"); // TEST #128
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' do /./ n set '#0-=1' endif", "Mismatched predicates: ENDIF at index 13 does not match WHILE (#0>0) at index 6"); // TEST #129
	VERIFY("a: /4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw endif", "4 (natural)"); // TEST #130
	VERIFY("a: /4/ 1 if 'a>=0' /(natural)/ nw else /(non-natural)/ nw endif", "Missing THEN after IF at index 4 with condition \"a>=0 (natural)\""); // TEST #131
	VERIFY("a: /4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw", "4 (natural)"); // TEST #132 - meaningless after issue #145
	VERIFY("a: /-4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw", "-4 (non-natural)"); // TEST #133 - meaningless after issue #145
	VERIFY("/4/ 1 endif","ENDIF without IF at index 3"); // TEST #134
	VERIFY("a: /4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw done", "Mismatched predicates: DONE at index 12 does not match IF (a>=0) at index 4"); // TEST #135

	VERIFY("requires version /4/ 1", "4");  // TEST #136
	VERIFY("requires hello /4/ 1", "Missing required configured literal <hello>"); // TEST #137

		// Default output placement
	spec = "hello 1";
	VERIFY(spec, "hello");                                                          // TEST #138

	spec = "hello";
	VERIFY(spec, "hello");                                                          // TEST #139

	spec = "if '1+1==2' then hello endif";
	VERIFY(spec, "hello");                                                         // TEST #140

	spec = "if '1+1==2' then hello endif bye";
	VERIFY(spec, "hello bye");                                                     // TEST #141

	spec = "if '1+1==3' then hello else goodbye endif Dolly";
	VERIFY(spec, "goodbye Dolly");                                               // TEST #142

	spec = "while '#0>1' do hello done bye";
	VERIFY(spec, "bye");                                                         // TEST #143

	// tf2mcs and mcs2tf
	VERIFY2("1-* tf2mcs %Y-%m-%dT%H:%M:%S.%6f a: ID a mcs2tf /%A, %B %drd, %Y; %M minutes past the %Hth hour/ 1", "2018-11-23T14:43:43.126573","Friday, November 23rd, 2018; 43 minutes past the 14th hour"); // Test #144
#ifdef WIN64
	VERIFY("/1545407296548900/ mcs2tf '%c' 1", "12/21/18 17:48:16");  // Test #145
	VERIFY("a: /1545407296548900/ . print 'a+3600000000' mcs2tf '%c' 1", "12/21/18 18:48:16");  // Test #146
#else
	VERIFY("/1545407296548900/ mcs2tf '%c' 1", "Fri Dec 21 17:48:16 2018");  // Test #145
	VERIFY("a: /1545407296548900/ . print 'a+3600000000' mcs2tf '%c' 1", "Fri Dec 21 18:48:16 2018");  // Test #146
#endif

#ifdef SPANISH_LOCALE_SUPPORTED
#ifdef PUT_TIME__SUPPORTED
	specTimeSetLocale("es_ES");
	VERIFY("/1545407296548900/ mcs2tf '%A,%d-%B-%Y' 1", "viernes,21-diciembre-2018");  // TEST #147
	specTimeSetLocale("C");
#else
	VERIFY("/1545407296548900/ mcs2tf '%A,%d-%B-%Y' 1", "Friday,21-December-2018");  // TEST #147
#endif
#endif
	VERIFY("/1545407296548900/ mcs2tf '%A,%d-%B-%Y' 1", "Friday,21-December-2018");  // TEST #148

	spec = "printonly eof  a: w1 1 set '#0+=a' eof print '#0' 1";
	VERIFY2(spec, "1\n2\n3\n4\n5", "15");                                            // TEST #149

	spec = "printonly a  a: w1 . w2 1 set '#0+=a' eof print '#0' 1";
	VERIFY2(spec, "1 1\n1 2\n1 3\n2 4\n2 5", "1\n4\n7");                             // TEST #150

	spec = "printonly a keep a: w1 . w2 nw set '#0+=a' eof print '#0' 1";
	VERIFY2(spec, "1 1\n1 2\n1 3\n2 4\n2 5", "1\n2 3 4\n7");                         // TEST #151

	spec = "a: w1 . skip-until 'a>2' id a";
	VERIFY2(spec, "1\n2\n3\n4\n5\n6", "3\n4\n5\n6");                                 // TEST #152

	spec = "a: w1 . skip-while 'a<3' id a";
	VERIFY2(spec, "1\n2\n3\n4\n5\n6", "3\n4\n5\n6");                                 // TEST #153

	spec = "a: w1 . skip-until 'a>2' set '#0 += a' skip-until 'a>4' GT 1 EOF print #0";
	VERIFY2(spec, "1\n2\n3\n4\n5\n6", "GT\nGT\n18");                                // TEST #154. 3+4+5+6=18

	spec = "a: w1 EOF print 'sum(a)'";
	VERIFY2(spec, "1\n2\n3\n4", "1\n2\n3\n4\n10");                                  // TEST #155. Elide before EOF

	spec = "a: w1 if 'a>2' then b: w1 EOF print 'sum(b)'";
	VERIFY2(spec, "1\n2\n3\n4", "1\n2\n3 3\n4 4\n7");                               // TEST #156. Elide endif and placement before EOF

	spec = "w1 1 EOF /hello/ 1 write /bye/ 1";
	VERIFY2(spec, "1\n2", "1\n2\nhello\nbye");                                      // TEST #157. Write

	spec =  "a: w1-* . "                  \
			"set '#0:=countocc(hot)'  "   \
			"set '#1:=countocc(cold)' "   \
			"number 1 "                   \
			"EOF "                        \
			"print 'countocc_get(hot)' 1 'hot and' nw print 'countocc_get(cold)' nw cold nw write " \
			"print 'countocc_dump(lin)'";
	strm = "hot \n"                   \
			"cold \n"                 \
			"hot & cold \n"           \
			"cold & hot \n"           \
			"neither hot nor cold \n" \
			"warm \n"                 \
			"uppercase HOT and lowercase cold";
	std::string res = \
			"         1\n"  \
			"         2\n"  \
			"         3\n"  \
			"         4\n"  \
			"         5\n"  \
			"         6\n"  \
			"         7\n"  \
			"4 hot and 5 cold\n" \
			"+------+---+\n"  \
			"| cold | 5 |\n"  \
			"| hot  | 4 |\n"  \
			"+------+---+\n";

	VERIFY2(spec, strm.c_str(), res); // TEST #158

	spec = "w1 a: if 'a%2=1'";
	VERIFY2(spec, "1\n2\n3\n4", "1\n3");    // TEST #159. Print entire record if the condition holds

	// Issue #203 - collecting additional tokens for conditions after if, elseif, and while
	
	spec = "w1 a: IF a%2=1 & a<3 THEN /a is one/ 1";
	VERIFY2(spec, "3\n2\n1", "a is one");     // TEST #160 - complex condition between if and then

	spec = "w1 a: IF a%2=1 & a<3 /a is one/ 1";
	VERIFY2(spec, "3\n2\n1", "Missing THEN after IF at index 3 with condition \"a%2=1 & a<3 a is one\"");     // TEST #161 - complex condition, but forgot the THEN

	spec = "SET #0:=word(1) WHILE #0%2=0 & #0>0 DO SET #0/=2 DONE print #0";
	VERIFY2(spec, "10\n9\n8", "5\n9\n1");   // TEST #162 - complex condition in WHILE

	spec = "w1 a: ASSERT a%2=1 | a>6 ID a 1";
	VERIFY2(spec, "8\n7\n6\n5\n4", "Bad output placement Token LITERAL at index 6 with content <a>6>");  // TEST #163

	spec = "w1 a: SKIP-UNTIL a<6 ID a 1";
	VERIFY2(spec, "8\n7\n6\n5\n4", "5\n4");  // TEST #164

	spec = "w1 a: SKIP-UNTIL a%2=1 & a<6 ID a 1";
	VERIFY2(spec, "8\n7\n6\n5\n4", "Bad output placement Token LITERAL at index 6 with content <a<6>");  // TEST #165

	// Issue 224
	spec = "3-* 1 REDO IF \"word(1)=='add'\"";
	VERIFY2(spec, "hello there\ngladd\ngood-bye", "add");     // TEST #166

	// Issue 227
	spec = "FIELDSEP _ SUBSTRING FIELD 3 of WORD 1";
	VERIFY2(spec, "a_b_c_d_e", "c");   // TEST #167
	spec = "WORDSEP _ SUBSTRING WORD 3 of FIELD 1";
	VERIFY2(spec, "a_b_c_d_e", "c");   // TEST #168

	// Compound set - Issue 223
	spec = "SET '(#0:=word(1))' print #0";
	VERIFY2(spec, "1\n2\n3\n4", "1\n2\n3\n4"); // TEST #169

	spec = "SET '(#0:=word(1);#1+=#0)' print #1";
	VERIFY2(spec, "1\n2\n3\n4", "1\n3\n6\n10"); // TEST #170

	spec = "SET #0:=word(1);#1+=#0 print #1";
	VERIFY2(spec, "1\n2\n3\n4", "1\n3\n6\n10"); // TEST #171

	spec = "SET '#0:=word(1);#1+=#0' print #1";
	VERIFY2(spec, "1\n2\n3\n4", "1\n3\n6\n10"); // TEST #172

	// Multi-char field and word separator
	VERIFY2("fs :. field 2 1 field 5 nw", "192.168.1.5:443", "168 443"); // Test #173
	VERIFY2("fs :. field 2 1 field 5 nw", "192.168.1.5:443.5", "168 443"); // Test #174
	VERIFY2("w1 1 substr fs :. field 3:-2 of w2 nw", "ip:port 192.168.1.5:443.5", "ip:port 1.5:443"); // Test #175

	VERIFY2("ws :. word 2 1 word 5 nw", "192.168.1.5:443", "168 443"); // Test #176
	VERIFY2("ws :. word 2 1 word 5 nw", "192.168.1:.:.5:443.5", "168 443"); // Test #177
	VERIFY2("w1 1 substr ws :. word 3:-2 of 9-* nw", "ip:port 192.168.1.5:443.5", "ip:port 1.5:443"); // Test #178

	// Issue #241
	VERIFY("print '99.2189254761+1'", "100.2189254761");  // Test #179

	// While-guard - similar to 104
	g_WhileGuardLimit = 100;
	spec = "while '#0<10000' do                  " \
           "   print 'fmap_sample(a,rand(10))' . " \
           "   set '#0+=1'                       " \
           "done                                 " \
           "set '#1:=fmap_count(a,7)'            " \
           "if '#1 > 900 & #1 < 1100' then       " \
           "   /OK/ 1                            " \
           "else                                 " \
           "   /NOT OK/ 1                        " \
           "endif                                ";

	VERIFY(spec, "Potentially endless while-loop detected in Token 2 with condition <#0<10000> - looped 101 times.");  // TEST #180

	spec = "SUBSTR WS @/@ WORD 2 of WORD 3";
	VERIFY2(spec, "The Epoch: 1/Jan/1970 at midnight", "Jan"); // Test #181

    spec = "SUBSTR WS @/@ FS i WORD 2 of WORD 3";
	VERIFY2(spec, "The Epoch: 1/Jan/1970 at midnight", "Jan"); // Test #182

	VERIFY2(spec, "The Epic 1/Jan/1970 at midnight", "Jan"); // Test #183

    spec = "SUBSTR WS @/@ FS i WORD 2 of FIELD 3";
	VERIFY2(spec, "The Epic: 1/Jan/1970 at midnight", ""); // Test #184

    spec = "SUBSTR WS @/@ WORD 3 of FS i FIELD 2";
	VERIFY2(spec, "The Epic: 1/Jan/1970 at midnight", "1970 at m"); // Test #185

    spec = "SUBSTR WS / WORD 3 of FS i FIELD 2";
	VERIFY2(spec, "The Epic: 1/Jan/1970 at midnight", "1970 at m"); // Test #186

	// RECNO - Issue #277
	spec = "WORD 1 a: IF a%2==0 THEN RECNO 1";
	VERIFY2(spec, "1\n2\n3\n4\n5", "         2\n         4"); // Test #187

	// SPLITW - basic word splitting
	VERIFY2("splitw 1", "one two three", "one\ntwo\nthree"); // Test #188
	VERIFY2("splitw", "one two three", "one\ntwo\nthree"); // Test #189 - elided output placement
	VERIFY2("splitw nextword", "one two three", "one\ntwo\nthree"); // Test #190

	// SPLITW with prefix
	VERIFY2("'prefix' 1 splitw nextword", "one two three", "prefix one\nprefix two\nprefix three"); // Test #191

	// SPLITW with REDO
	VERIFY2("splitw 1 redo 'WORD:' 1 1-* next", "the boy went", "WORD:the\nWORD:boy\nWORD:went"); // Test #192

	// SPLITW with custom separator
	VERIFY2("splitw ws , 1", "a,b,c", "a\nb\nc"); // Test #193

	// SPLITW with OF range (word range)
	VERIFY2("splitw of w2-3 1", "one two three four", "two\nthree"); // Test #194

	// SPLITF - basic field splitting
	VERIFY2("fs : splitf 1", "a:b:c", "a\nb\nc"); // Test #195

	// SPLITF with prefix
	VERIFY2("fs : 'F:' 1 splitf nextword", "x:y:z", "F: x\nF: y\nF: z"); // Test #196

	// SPLITF with custom separator
	VERIFY2("splitf fs , 1", "a,b,c", "a\nb\nc"); // Test #197

	// SPLITF with empty fields
	VERIFY2("fs : splitf 1", "a::c", "a\n\nc"); // Test #198

	// SPLITW with range output placement (width-constrained)
	VERIFY2("splitw 1-5", "one two three", "one  \ntwo  \nthree"); // Test #199

	// SPLITW single word - produces one record
	VERIFY2("splitw 1", "hello", "hello"); // Test #200


	// Error: nested splits
	VERIFY2("splitw 1 splitf 1", "test", "Nested SPLITW/SPLITF is not allowed at index 3"); // Test #201

	// Error: mismatched separator - SPLITW with FS
	VERIFY2("splitw fs x 1", "test", "SPLITW cannot be followed by FIELDSEPARATOR at index 2"); // Test #202

	// Error: mismatched separator - SPLITF with WS
	VERIFY2("splitf ws x 1", "test", "SPLITF cannot be followed by WORDSEPARATOR at index 2"); // Test #203

	// Bounds checking tests for WHILE and IF as last tokens
	VERIFY("while", "Missing DO after WHILE at index 1"); // Test #204
	VERIFY("if", "Missing THEN after IF at index 1"); // Test #205

	// REDO output validation
	VERIFY("w1 1 REDO", "REDO at index 3 must be followed by output-producing spec units"); // Test #206
	VERIFY("REDO w1 1", "REDO at index 1 must be preceded by output-producing spec units"); // Test #207
	VERIFY("w1 a: REDO w1 1", "REDO at index 3 must be preceded by output-producing spec units"); // Test #208
	VERIFY("SPLITW REDO w1 1", "The\nquick\nbrown\nfox\njumped\nover\nthe\nlazy\ndog"); // Test #209
	VERIFY("EOF print '2+2'", "EOF at index 1 must be preceded by data-providing spec units"); // Test #210
	VERIFY("w1 a: EOF", "EOF at index 3 must be followed by output-producing spec units"); // Test #211
	VERIFY2("w1 a: EOF print 'sum(a)'", "1\n2\n3\n4", "10"); // Test #212
	VERIFY("w1 1 REDO w1 a: REDO hello", "REDO at index 6 must be preceded by output-producing spec units"); // Test #213
	VERIFY("w1 1 REDO EOF bye", "EOF at index 4 must be preceded by data-providing spec units"); // Test #214
	VERIFY("w1 REDO w1 1", "The"); // Test #215
	VERIFY2("w1 1 REDO set '#0:=7' EOF print '#0'", "x", "7"); // Test #216

	// rand() with non-positive limit
	VERIFY2("print 'rand(0)' 1", "x", "rand: limit must be a positive integer"); // Test #217
	VERIFY2("print 'rand(-5)' 1", "x", "rand: limit must be a positive integer"); // Test #218

	// Security fix regression tests (Issue #336)
	// strip() on all-whitespace string (was crash due to npos in substr)
	VERIFY2("print 'strip(\"   \",\"B\")' 1", "x", ""); // Test #219

	// sword() with negative count on all-separator string (was pointer underflow)
	VERIFY2("print 'sword(\"xxx\",-1,\"x\")' 1", "x", ""); // Test #220

	// sfield() with out-of-range negative count (was pointer underflow)
	VERIFY2("print 'sfield(\"a\",-2,\",\")' 1", "x", ""); // Test #221

	// fact() argument exceeding 64-bit overflow limit (was silent overflow)
	VERIFY2("print 'fact(21)' 1", "x", "fact: argument too large (max 20 for 64-bit integers)"); // Test #222
	
	// STRIP modifier on all-whitespace input (was out-of-bounds read in stripString)
	VERIFY2("1-* strip 1", "   ", ""); // Test #223

	// Exactness of statistical functions with integer input
	spec =  "a: WORD 1 .                               " \
			" EOF                                      " \
			"   PRINT 'exact(sum(a))'                 1" \
			"   PRINT 'exact(min(a))'                NW" \
			"   PRINT 'exact(max(a))'                NW";
	VERIFY2(spec, "1\n2\n3\n4\n5", "1 1 1"); // TEST #224

	// average of a single integer should be exact
	spec =  "a: WORD 1 . EOF PRINT 'exact(average(a))' 1";
	VERIFY2(spec, "42", "1"); // TEST #225

	// average of multiple integers is inexact (division)
	spec =  "a: WORD 1 . EOF PRINT 'exact(average(a))' 1";
	VERIFY2(spec, "1\n2\n3", "0"); // TEST #226

	// variance, stddev, stderrmean are always inexact
	spec =  "a: WORD 1 .                               " \
			" EOF                                      " \
			"   PRINT 'exact(variance(a))'            1" \
			"   PRINT 'exact(stddev(a))'             NW" \
			"   PRINT 'exact(stderrmean(a))'         NW";
	VERIFY2(spec, "1\n2\n3\n4\n5", "0 0 0"); // TEST #227

	// Statistical functions with float input are inexact
	spec =  "a: WORD 1 .                               " \
			" EOF                                      " \
			"   PRINT 'exact(sum(a))'                 1" \
			"   PRINT 'exact(min(a))'                NW" \
			"   PRINT 'exact(max(a))'                NW";
	VERIFY2(spec, "1.5\n2.5\n3.5", "0 0 0"); // TEST #228

	// === Rolling Context tests ===

	// CONTEXT 0 resets to current record
	spec = "CONTEXT 0 1-* 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\nbeta\ngamma"); // TEST #229

	// CONTEXT +1 peeks at next record
	spec = "1-* 1 CONTEXT 1 1-* NW";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha beta\nbeta gamma\ngamma"); // TEST #230

	// CONTEXT -1 peeks at previous record
	spec = "1-* 1 CONTEXT -1 1-* NW";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\nbeta alpha\ngamma beta"); // TEST #231

	// @+1 in expression peeks at next record
	spec = "print @+1 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "beta\ngamma\n"); // TEST #232

	// @-1 in expression peeks at previous record
	spec = "print @-1 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "\nalpha\nbeta"); // TEST #233

	// @+0 is the same as @@
	spec = "print @+0 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\nbeta\ngamma"); // TEST #234

	// @-0 is the same as @@
	spec = "print @-0 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\nbeta\ngamma"); // TEST #235

	// CONTEXT with larger forward offset
	spec = "1-* 1 CONTEXT 2 1-* NW";
	VERIFY2(spec, "A\nB\nC\nD\nE", "A C\nB D\nC E\nD\nE"); // TEST #236

	// CONTEXT with larger backward offset
	spec = "1-* 1 CONTEXT -2 1-* NW";
	VERIFY2(spec, "A\nB\nC\nD\nE", "A\nB\nC A\nD B\nE C"); // TEST #237

	// Combined forward and backward in one spec
	spec = "CONTEXT -1 1-* 1 CONTEXT 0 1-* NW CONTEXT 1 1-* NW";
	VERIFY2(spec, "A\nB\nC", "A B\nA B C\nB C"); // TEST #238

	// @+n in expression with function
	spec = "PRINT 'length(@+1)' 1";
	VERIFY2(spec, "AB\nCDE\nF", "3\n1\n0"); // TEST #239

	// Out-of-range forward context returns empty string
	spec = "print @+5 1";
	VERIFY2(spec, "only", ""); // TEST #240

	// Out-of-range backward context returns empty string
	spec = "print @-5 1";
	VERIFY2(spec, "only", ""); // TEST #241

	// @@ returns the real input record, not the CONTEXT-modified one
	spec = "CONTEXT -1 PRINT '@@' 1 WRITE PRINT '@-1' 1 WRITE";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\n\nbeta\nalpha\ngamma\nbeta"); // TEST #242

	// === ctxrecno tests ===

	// ctxrecno without CONTEXT returns same as recno
	spec = "PRINT 'ctxrecno()' 1";
	VERIFY2(spec, "a\nb\nc", "1\n2\n3"); // TEST #243

	// ctxrecno with CONTEXT 1 returns recno + 1
	spec = "CONTEXT 1 PRINT 'ctxrecno()' 1";
	VERIFY2(spec, "a\nb\nc", "2\n3\n4"); // TEST #244

	// ctxrecno with CONTEXT -1 returns recno - 1
	spec = "CONTEXT -1 PRINT 'ctxrecno()' 1";
	VERIFY2(spec, "a\nb\nc", "0\n1\n2"); // TEST #245

	// ctxrecno with CONTEXT 0 returns same as recno
	spec = "CONTEXT 0 PRINT 'ctxrecno()' 1";
	VERIFY2(spec, "a\nb\nc", "1\n2\n3"); // TEST #246

	// ctxrecno resets after CONTEXT changes
	spec = "PRINT 'ctxrecno()' 1 CONTEXT 1 PRINT 'ctxrecno()' NW";
	VERIFY2(spec, "a\nb\nc", "1 2\n2 3\n3 4"); // TEST #247

	// EOF token should not terminate processing during runout cycle
	// when bNeedRunoutCycleFromStart is set by eof() in a condition
	spec = "w1 a: EOF if /eof()/ then /hello/ 1 endif";
	VERIFY2(spec, "test", "hello"); // TEST #248

	// Same with visible pre-EOF output
	spec = "a: w1 1 EOF if /eof()/ then /done/ 1 endif";
	VERIFY2(spec, "x\ny", "x\ny\ndone"); // TEST #249

	// CONTEXT + EOF + eof(): CONTEXT changes m_ps during runout,
	// but eof() should still return true and EOF token should not stop processing
	spec = "w1 1 CONTEXT 1 if /!eof()/ then 1-* nw endif EOF if /eof()/ then /RUNOUT/ 1 endif";
	VERIFY2(spec, "a\nb\nc", "a b\nb c\nc\nRUNOUT"); // TEST #250

	// @! returns the context-affected record (same as record() or 1-*)
	// Without CONTEXT, @! and @@ are equivalent
	spec = "PRINT '@!' 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\nbeta\ngamma"); // TEST #251

	// With CONTEXT, @! returns the context-affected record while @@ returns the original
	spec = "CONTEXT 1 PRINT '@!' 1 WRITE PRINT '@@' 1 WRITE";
	VERIFY2(spec, "alpha\nbeta\ngamma", "beta\nalpha\ngamma\nbeta\n\ngamma"); // TEST #252

	// @! with CONTEXT -1 returns the previous record
	spec = "CONTEXT -1 PRINT '@!' 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "\nalpha\nbeta"); // TEST #253

	// cfrecord() without CONTEXT returns the same as record()
	spec = "PRINT 'cfrecord()' 1";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha\nbeta\ngamma"); // TEST #254

	// cfrecord() with CONTEXT returns the original input record (not context-affected)
	spec = "CONTEXT 1 PRINT 'cfrecord()' 1 PRINT 'record()' NW";
	VERIFY2(spec, "alpha\nbeta\ngamma", "alpha beta\nbeta gamma\ngamma"); // TEST #255

	// record(), word(), field(), range() return empty string during forced run-out cycle
	spec = "PRINT 'record()' 1 PRINT 'eof()' NEXTWORD";
	VERIFY2(spec, "hello\nworld", "hello 0\nworld 0\n1"); // TEST #256

	spec = "PRINT 'word(1)' 1 PRINT 'eof()' NEXTWORD";
	VERIFY2(spec, "hello world", "hello 0\n1"); // TEST #257

	spec = "PRINT 'field(1)' 1 PRINT 'eof()' NEXTWORD";
	VERIFY2(spec, "hello\tworld", "hello 0\n1"); // TEST #258

	spec = "PRINT 'range(1,3)' 1 PRINT 'eof()' NEXTWORD";
	VERIFY2(spec, "abcdef", "abc 0\n1"); // TEST #259
	
	// All the ways of accessing a record with and without CONTEXT
	spec = "'Cycle:' 1 PRINT 'recno()' WRITE "                   \
		"   'Context:' 3 PRINT 'ctxrecno()' WRITE"               \
		"   'Using Spec Units:' 5 1-*                25 WRITE"   \
		"   'Using record():'   5 PRINT 'record()'   25 WRITE"   \
		"   'Using @@:'         5 PRINT '@@'         25 WRITE"   \
		"   'Using @!:'         5 PRINT '@!'         25 WRITE"   \
		"   'Using cfrecord():' 5 PRINT 'cfrecord()' 25 WRITE"   \
		"   'Setting CONTEXT to +1' 3 CONTEXT +1   WRITE"        \
		"   'Context:' 3 PRINT 'ctxrecno()' WRITE"               \
		"   'Using Spec Units:' 5 1-*                25 WRITE"   \
		"   'Using record():'   5 PRINT 'record()'   25 WRITE"   \
		"   'Using @@:'         5 PRINT '@@'         25 WRITE"   \
		"   'Using @!:'         5 PRINT '@!'         25 WRITE"   \
		"   'Using cfrecord():' 5 PRINT 'cfrecord()' 25 WRITE"   \
		"   'Setting CONTEXT to -1' 3 CONTEXT -1   WRITE"        \
		"   'Context:' 3 PRINT 'ctxrecno()' WRITE"               \
		"   'Using Spec Units:' 5 1-*                25 WRITE"   \
		"   'Using record():'   5 PRINT 'record()'   25 WRITE"   \
		"   'Using @@:'         5 PRINT '@@'         25 WRITE"   \
		"   'Using @!:'         5 PRINT '@!'         25 WRITE"   \
		"   'Using cfrecord():' 5 PRINT 'cfrecord()' 25 WRITE";
	strm = "Wise men say\nOnly fools rush in\nBut I can't help falling in love with you";
	res = \
		"Cycle: 1\n"  \
		"  Context: 1\n"  \
		"    Using Spec Units:   Wise men say\n"  \
		"    Using record():     Wise men say\n"  \
		"    Using @@:           Wise men say\n"  \
		"    Using @!:           Wise men say\n"  \
		"    Using cfrecord():   Wise men say\n"  \
		"  Setting CONTEXT to +1\n"  \
		"  Context: 2\n"  \
		"    Using Spec Units:   Only fools rush in\n"  \
		"    Using record():     Only fools rush in\n"  \
		"    Using @@:           Wise men say\n"  \
		"    Using @!:           Only fools rush in\n"  \
		"    Using cfrecord():   Wise men say\n"  \
		"  Setting CONTEXT to -1\n"  \
		"  Context: 0\n"  \
		"    Using Spec Units:   \n"  \
		"    Using record():     \n"  \
		"    Using @@:           Wise men say\n"  \
		"    Using @!:           \n"  \
		"    Using cfrecord():   Wise men say\n"  \
		"Cycle: 2\n"  \
		"  Context: 2\n"  \
		"    Using Spec Units:   Only fools rush in\n"  \
		"    Using record():     Only fools rush in\n"  \
		"    Using @@:           Only fools rush in\n"  \
		"    Using @!:           Only fools rush in\n"  \
		"    Using cfrecord():   Only fools rush in\n"  \
		"  Setting CONTEXT to +1\n"  \
		"  Context: 3\n"  \
		"    Using Spec Units:   But I can't help falling in love with you\n"  \
		"    Using record():     But I can't help falling in love with you\n"  \
		"    Using @@:           Only fools rush in\n"  \
		"    Using @!:           But I can't help falling in love with you\n"  \
		"    Using cfrecord():   Only fools rush in\n"  \
		"  Setting CONTEXT to -1\n"  \
		"  Context: 1\n"  \
		"    Using Spec Units:   Wise men say\n"  \
		"    Using record():     Wise men say\n"  \
		"    Using @@:           Only fools rush in\n"  \
		"    Using @!:           Wise men say\n"  \
		"    Using cfrecord():   Only fools rush in\n"  \
		"Cycle: 3\n"  \
		"  Context: 3\n"  \
		"    Using Spec Units:   But I can't help falling in love with you\n"  \
		"    Using record():     But I can't help falling in love with you\n"  \
		"    Using @@:           But I can't help falling in love with you\n"  \
		"    Using @!:           But I can't help falling in love with you\n"  \
		"    Using cfrecord():   But I can't help falling in love with you\n"  \
		"  Setting CONTEXT to +1\n"  \
		"  Context: 4\n"  \
		"    Using Spec Units:   \n"  \
		"    Using record():     \n"  \
		"    Using @@:           But I can't help falling in love with you\n"  \
		"    Using @!:           \n"  \
		"    Using cfrecord():   But I can't help falling in love with you\n"  \
		"  Setting CONTEXT to -1\n"  \
		"  Context: 2\n"  \
		"    Using Spec Units:   Only fools rush in\n"  \
		"    Using record():     Only fools rush in\n"  \
		"    Using @@:           But I can't help falling in love with you\n"  \
		"    Using @!:           Only fools rush in\n"  \
		"    Using cfrecord():   But I can't help falling in love with you";
	VERIFY2(spec, strm.c_str(), res.c_str());   // TEST #260

	// ctxoffset() function test
	spec = "PRINT 'ctxoffset()' 1  CONTEXT +1  PRINT 'ctxoffset()' NW  CONTEXT -1  PRINT 'ctxoffset()' NW";
	VERIFY2(spec, "x", "0 1 -1"); // TEST #261

	// ctxoob() function test - CONTEXT-based
	spec = "PRINT 'ctxoob()' 1  CONTEXT +1  PRINT 'ctxoob()' NW  CONTEXT -1  PRINT 'ctxoob()' NW";
	VERIFY2(spec, "x", "0 1 1"); // TEST #262

	// ctxoob() function test - @± expression-based
	spec = "PRINT 'ctxoob(@@)' 1  PRINT 'ctxoob(@+1)' NW  PRINT 'ctxoob(@-1)' NW";
	VERIFY2(spec, "x", "0 1 1"); // TEST #263

	if (errorCount) {
		std::cout << '\n' << errorCount << '/' << testCount << " tests failed.\n";
		std::cout << "Failed tests: ";
		for (int i : failedTests) {
			std::cout << i << " ";
		}
		std::cout << "\n";
	} else {
		std::cout << "\n*** All tests passed.\n";
	}

	return (errorCount==0) ? 0 : 4;
}
