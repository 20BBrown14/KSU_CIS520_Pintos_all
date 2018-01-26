README.txt

Branden Brown
Project 0
Jan 19, 2017

To incorporate the "alarm-mega" test I executed ' grep -r alarm-multiple * ' and ' grep -r alarm_multiple * ' commands
that allowed me to see what files included the phrase "alarm_multiple" or "alarm-multiple." I did this because I was
just going to copy the alarm-multiple to make alarm-mega.

I had to create a new file, ' alarm-mega.ck ' in pintos/src/tests/threads which just the alarm-multiple.ck file but the 
7 in the last line is changed to 70.

In tests.c, in the same directory, I added the line: {"alarm-mega", test_alarm_mega}, to the tests struct.

In tests.h, in the same directory, I added the line: extern test_func test_alarm_mega; to the tests_func typedef.

In alarm-wait.c, in the same directory, I added the function:
void
test_alarm_mega (void)
{
	test_sleep (5, 70);
}

which is just a copy of the already existing test_alarm_multiple function with the 7 changed to a 70.

In Rubric.alarm, in the same directory, I added the line: 4	alarm-mega

In Make.tests, in the same directory, I added ' alarm-mega ' to the test names.

With all the done, and running the make command again, the alarm-mega test worked.