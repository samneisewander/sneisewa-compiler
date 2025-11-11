# BMinor Compiler Development Log

## Assignment Questions

### Resolver

> What AI tools (if any) did you use, and what sort of prompts did you provide?
Didn't use AI.
> What parts of the code were easy to get right?
This one wasn't actually that hard. The various resolve functions were clearly described in the textbook, and were pretty simple to implement.

> What parts were difficult to get right and required more effort?
Debugging was the hardest part. I had one issue where my global stack was getting overwritten, and it was hard to determine where that was happening without staring at GDB for a long, long time. Turned out that I had forgetten to `break` a case in my switch statement.


### Printer

> What AI tools (if any) did you use, and what sort of prompts did you provide?

I didn't use AI tools.

> What parts of the code were easy to get right?

It was all hard.

> What parts were difficult to get right and required more effort?

All of it. I spent probably 15 hours on this homework total, and couldn't complete everything. I don't understand why it isn't working. I don't even know how to approach working out the parentheses or fixing the spacing.

### Scanner

> Update your development log to indicate what tools you used, what worked well, and what was challenging.

In terms of tools, I used [regex101.com](https://regex101.com) and the `Flex` manual. I also discussed some behavior and edge casses with my friends.

What worked well was extending my existing code. It was really easy to add more test cases, update the Makefile, handle the new `--scan` flag, and integrate the scanner with the decoder. It was also pretty easy getting the Flex workflow going once I read and understood the syntax.

The tricky part was getting all of the tokens! There were a bunch I didn't think of, like `:` and `;` and `,`. It was also tricky getting the regular expressions right for eating comments and correctly handling the different atomic types. The way strings behave in this year's spec is still really strange to me, and I feel as though its going to cause some very weird behavior downstream.

It was also challenging coming up with thorough test cases for this one. The werid thing is that the syntax doesn't actually have to be correct; any tokens can appear in any order as long as they're all valid. I have a sinking feeling that there is some error the scanner could be catching that I did not think of, and that I did not write test cases for.

### Encoder

#### Note about hex escape codes
My decoder treats an input like `"\0xHH"` as valid, treating the `\0` as a regular `0` character per the specification:
> Any other character following a backslash represents exactly that character.

So the encoder would decode this example as `0xHH`.

#### What AI tools (if any) did you use, and what sort of prompts did you provide?
I mostly looked at the outputs of the AI overview while I was Googling. I was hoping to find Stack Overflow articles to find strategies for solving these problems, but the AI overview was more convenient. Here were some searches I made:
- shell repeat char n times
- gdb print string
- where are EXIT_SUCCESS and EXIT_FAILURE defined
- touch several files at once

#### What parts of the code were easy to get right?
The test harness wasn't that difficult to set up, and the encoder function was also pretty simple given that I had already completed the decoder. It also wasn't terribly hard to make the Makefile.

#### What parts were difficult to get right and required more effort?
The decoder was tricky. I had to rethink my testcases and my code several times, and I noticed that sometimes my testcases would debug the decoder but other times the decoder would debug the test cases. I also found the testing script to be frustrating, because I got stuck with some weirdness with I/O redirection. There was a moment when I thought my encoder was working correctly for the `bad[n].bminor` testcases but in reality the program was segfaulting and the output was supressed. Another especially tricky part was handling the hex escape codes, since the conversion from ASCII to hex is a little tricky.

One other thing I noticed was the large, slow switch statement in the decoder, which might be replaced by a hash table in a faster implementation. I may end up going back to correct that (or even time the decoder before and after and calculate the speedup), but for now this implementation works and doesn't *need* to be improved.

## Encoder / Decoder Notes

### Makefile

Gemini suggested echoing the current recipe in the Makefile, so I add that in:
```Makefile
test:
	@echo "Running tests..."
```

I noticed that the provided testing script has a bug, which is corrected in the following code snippet:
```sh
#!/bin/sh

for testfile in test/encode/good*.bminor
do
	if bminor --encode $testfile > $testfile.out
	then
		echo "$testfile success (as expected)"
	else
		echo "$testfile failure (INCORRECT)"
	fi
done

for testfile in test/encode/bad*.bminor
do
	---if bminor --encode test/encode/$testfile > $testfile.out---
    +++if bminor --encode $testfile > $testfile.out+++
	then
		echo "$testfile success (INCORRECT)"
	else
		echo "$testfile failure (as expected)"
	fi
done
```

### Writing tests

The specification says:
> Both char and string literals may contain any ASCII value between 32 and 127. 

I wasn't initially sure if that *included* 32 and 127, but after asking I learned that it does include those values. This led to a second, question: how do I test that the decoder is allowing the `DEL` character? The solution is to use a small C script to print control characters to a text file.

```c
fprintf(out, "\"%c <- that is a terrible character\"", 127);
```

Here's another question I had: if the decoder takes a string like `"hello" "world"`, what would happen? I imagined it could either accept the first string `"hello"` and truncate the rest, or it would reject the string since it contains unescaped internal double quotes.

After speaking with a TA, I learned that the decoder should determine whether the *entire input* is a valid string or not, which means that the second case above is the desired behavior. This also means that leading or trailing whitespace in the test cases causes the test to fail. To do this, I adjusted my decoder to continue reading characters until it encounters a second `"` character, then check that the character immediately following that is a null terminator. If not, then this string is invalid.

Here are some error cases I thought to consider:
- missing quotes
- illegal double quote in string body
- illegal characters, unicode stuff like emojis
- newlines that arent escaped
- too many characters
- make sure all legal characters work
- make sure all escape codes work
- multiple correct strings in line
- closing double quotes is escaped
- hex escape code is not a hec digit >>> maybe not this one lol
- allow empty strings

### string_decode

- scanning hex codes
- converting ascii range to decimal digits
- converting decimal digits to hexadecimal digits
- handling escape sequences
- can't test that the hex escape codes work with this test harness

### string_encode

This function was easy to implement once `string_decode` was working. Here are a few tricks I used:

I used post-increment frequently to update and move the character pointers at the same time.
```C
*(es++) = '"';
```

I learned that you can use an existing variable in a for loop and just skip the initializer step:
```C
for (; *s; s++) {
	/* ... */
}
```

### AI

I learned the following tricks by chatting with Gemini:

#### Phony directive
You can add a directive to a Makefile like this: 
```Makefile
.PHONY: test
```

This tells the makefile that the recipe for `test` should run regardless of whether the targets are up to date or not. This fixed an issue I was having where I would go to run my test and make would not run the `test` recipe because all of the files were up to date. 

#### Brace expansion
This was kind of a review of concepts in Systems. I learned that you can use brace expansion to run a command on several files in a range, like this:

```sh
touch bad{0..9}.bminor && touch good{0..9}.bminor
```

This saves some time when dealing with procedurally named files.

#### Repeat a character n times
I learned that you can print a single character to stdout `n` times like this:
```sh
printf "%255s\n" | tr ' ' 'a'
```

## Scanner Notes

Some cool regex:
- `\/\*(\*[^\/]|[^\*]\/|[^\*\/])*\*\/`: C-style comments
- `\/\/[^\n]*`: C++ comments
- `[ !\"\#$\%&'\(\)\*\+,\-\.\/:;<=>?@\[\]\^_`\{|\}~]`: every weird character in Bminor

## Parser Notes

Wow. This was incredibly hard.

> What AI tools (if any) did you use, and what sort of prompts did you provide?
I did not use any AI tools, but I did heavily reference the documentation for Bison. I also learned some neat tricks from the docs and from my friends, such as being able to print line numbers on parse failures or get more verbose error messages. 

> What parts of the code were easy to get right?
The order of operations stuff was not too bad, and getting Bison to play nicely with Flex and `bminor.c` wasn't that hard.  

> What parts were difficult to get right and required more effort?
Dangling else was a huge nightmare, and then getting the for loops to work on top of that was even worse. Then, being able to initialize arbitarily high-dimensional arrays was very hard also. At the moment, my head hurts to much to really think of other things that were challenging, but the assignment was extremly confusing and brutal from start to finish.  

It was also really hard to diagnose shift/reduce and reduce/reduce conflicts, even with Bison's help with counterexamples. Most of the time I had to resort to tracing through the parse on paper to reason about what when wrong and what I needed to change.