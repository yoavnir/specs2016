#include <iostream>
#include <string>
#include <string.h>
#include <iomanip>
#include "utils/platform.h"
#include "cli/tokens.h"
#include "specitems/specItems.h"
#include "processing/Config.h"
#include "processing/ProcessingState.h"
#include "processing/StringBuilder.h"

extern ALUCounters g_counters;

#define VERIFY(sp,ex) do {          \
		testCount++;                            \
		if (onlyTest!=0 && onlyTest != testCount) break;  \
		PSpecString ps = runTestOnExample(sp, "The quick brown fox jumped over the   lazy dog");  \
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testCount << " ";     \
		if (!ps) {                              \
			std::cout << "*** NOT OK ***: Got (NULL); Expected: <" << ex << ">\n"; \
			errorCount++;                       \
		} else {                                \
			if (ps->Compare(ex)) {              \
				std::cout << "*** NOT OK ***:\n\tGot <" << ps->data() << ">\n\tExp <" << ex << ">\n"; \
				errorCount++;                   \
			} else {                            \
				std::cout << "***** OK *****: <" << ex << ">\n"; \
			}                                   \
		}                                       \
		delete ps;								\
} while (0);

#define VERIFY2(sp,ln,ex) do {          \
		testCount++;                            \
		if (onlyTest!=0 && onlyTest != testCount) break;  \
		PSpecString ps = runTestOnExample(sp, ln);  \
		std::cout << "Test #" << std::setfill('0') << std::setw(3) << testCount << " ";     \
		if (!ps) {                              \
			std::cout << "*** NOT OK ***: Got (NULL); Expected: <" << ex << ">\n"; \
			errorCount++;                       \
		} else {                                \
			if (ps->Compare(ex)) {              \
				std::cout << "*** NOT OK ***:\n\tGot <" << ps->data() << ">\n\tExp <" << ex << ">\n"; \
				errorCount++;                   \
			} else {                            \
				std::cout << "***** OK *****: <" << ex << ">\n"; \
			}                                   \
			delete ps;			\
		}                                       \
} while (0);

#define VERIFYCMD(cmd,res) do {                 \
	std::string actual_res("");                 \
	testCount++;                                \
	std::cout << "Test #" << std::setfill('0') << std::setw(3) << testCount << " ";     \
	try { cmd; }                                \
	catch(SpecsException& e) {                  \
		actual_res = e.what(true);              \
	}                                           \
	if (res==actual_res) {                      \
		std::cout << "***** OK *****: <" << actual_res << ">\n"; \
	} else {                                    \
		errorCount++;                           \
		std::cout << "*** NOT OK ***:\n\tGot <" << actual_res << ">\n\tExp <" << res << ">\n"; \
	}                                           \
} while (0);


PSpecString runTestOnExample(const char* _specList, const char* _example)
{
	ProcessingState ps;
	ProcessingStateFieldIdentifierGetter fiGetter(&ps);
	setFieldIdentifierGetter(&fiGetter);
	setStateQueryAgent(&ps);
	classifyingTimer tmr;

	g_counters.clearAll();

	TestReader tRead(20);
	char* example = strdup(_example);
	char* ln = strtok(example, "\n");
	while (ln) {
		tRead.InsertString(ln);
		ln = strtok(NULL, "\n");
	}

	char* specList = (char*)_specList;

	std::vector<Token> vec = parseTokens(1, &specList);
	normalizeTokenList(&vec);
	itemGroup ig;

	unsigned int index = 0;
	try {
		ig.Compile(vec,index);
	} catch (const SpecsException& e) {
		return SpecString::newString(e.what(true));
	}

	StringBuilder sb;
	setPositionGetter(&sb);

	PSpecString result = NULL;
	try {
		if (ig.readsLines() || !ig.needRunoutCycle()) {
			do {
				PSpecString pFirstLine = tRead.getNextRecord();
				ps.setString(pFirstLine);
				ps.setFirst();
				ps.incrementCycleCounter();
				ig.processDo(sb, ps, &tRead, tmr);
				PSpecString pOut = sb.GetStringUnsafe();
				if (ps.shouldWrite()) {
					if (result) {
						result->add(pOut);
						delete pOut;
					} else {
						result = pOut;
					}
				} else {
					delete pOut;
				}
			} while (!tRead.endOfSource());
		}
	} catch (SpecsException& e) {
		if (result) {
			delete result;
		}
		result = SpecString::newString(e.what(true));
		goto end;
	}

	if (ig.needRunoutCycle()) {
		if (!ig.readsLines()) {
			ig.setRegularRunAtEOF();
		}
		ps.setString(NULL);
		ps.setFirst();
		try {
			ig.processDo(sb, ps, NULL, tmr);
			PSpecString pOut = sb.GetStringUnsafe();
			if (result) {
				result->add(pOut);
				delete pOut;
			} else {
				result = pOut;
			}
		} catch (SpecsException& e) {
			result = SpecString::newString(e.what(true));
			goto end;
		}
	}

end:
	free(example);
	while (!vec.empty()) {
		vec[0].deallocDynamic();
		vec.erase(vec.begin());
	}
	return result ? result : SpecString::newString();
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

	std::string spec;

	if (argc > 1) onlyTest = std::stoi(argv[1]);

	readConfigurationFile();

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

	// tf2d and d2tf
	VERIFY2("1-* tf2d %Y-%m-%dT%H:%M:%S.%6f a: ID a d2tf /%A, %B %drd, %Y; %M minutes past the %Hth hour/ 1", "2018-11-23T14:43:43.126573","Friday, November 23rd, 2018; 43 minutes past the 14th hour"); // Test #91
#ifdef WIN64
	VERIFY("/1545407296.548900/ d2tf '%c' 1", "12/21/18 17:48:16");  // Test #92
	VERIFY("a: /1545407296.548900/ . print 'a+3600' d2tf '%c' 1", "12/21/18 18:48:16");  // Test #93
#else
	VERIFY("/1545407296.548900/ d2tf '%c' 1", "Fri Dec 21 17:48:16 2018");  // Test #92
	VERIFY("a: /1545407296.548900/ . print 'a+3600' d2tf '%c' 1", "Fri Dec 21 18:48:16 2018");  // Test #93
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
	VERIFYCMD(specTimeSetLocale("kuku"),"Invalid locale <kuku>");  // TEST #111

#ifdef SPANISH_LOCALE_SUPPORTED
	VERIFYCMD(specTimeSetLocale("es_ES"),"");  // TEST #112
#ifdef PUT_TIME__SUPPORTED
	VERIFY("/1545407296.548900/ d2tf '%A,%d-%B-%Y' 1", "viernes,21-diciembre-2018");  // TEST #113
#else
	VERIFY("/1545407296.548900/ d2tf '%A,%d-%B-%Y' 1", "Friday,21-December-2018");  // TEST #113
#endif
#endif
	VERIFYCMD(specTimeSetLocale("C"),"");  // TEST #114
	VERIFY("/1545407296.548900/ d2tf '%A,%d-%B-%Y' 1", "Friday,21-December-2018");  // TEST #115
	
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
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' /./ n set '#0-=1' done", "Missing DO after WHILE at index 6 with condition \"#0>0\""); // TEST #126
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' do /./ n set '#0-=1'", "Predicate WHILE (#0>0) at index 6 is not terminated"); // TEST #127
	VERIFY("/4/ 1 done","DONE without WHILE at index 3"); // TEST #128
	VERIFY("a: /4/ 1 set '#0:=a' while '#0>0' do /./ n set '#0-=1' endif", "Mismatched predicates: ENDIF at index 13 does not match WHILE (#0>0) at index 6"); // TEST #129
	VERIFY("a: /4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw endif", "4 (natural)"); // TEST #130
	VERIFY("a: /4/ 1 if 'a>=0' /(natural)/ nw else /(non-natural)/ nw endif", "Missing THEN after IF at index 4 with condition \"a>=0\""); // TEST #131
	VERIFY("a: /4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw", "Predicate IF (a>=0) at index 4 is not terminated"); // TEST #132
	VERIFY("a: /-4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw", "Predicate IF (a>=0) at index 4 is not terminated"); // TEST #133
	VERIFY("/4/ 1 endif","ENDIF without IF at index 3"); // TEST #134
	VERIFY("a: /4/ 1 if 'a>=0' then /(natural)/ nw else /(non-natural)/ nw done", "Mismatched predicates: DONE at index 12 does not match IF (a>=0) at index 4"); // TEST #135

	if (errorCount) {
		std::cout << '\n' << errorCount << '/' << testCount << " tests failed.\n";
	} else {
		std::cout << "\nAll tests passed.\n";
	}

	return (errorCount==0) ? 0 : 4;
}
