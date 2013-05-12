import string, datetime,llist 
from llist import LinkedList as LList

print "Example of a static function::"
test_str = 'here\'s a string to reverse'
print llist.firstfnc(test_str,'r')
print "First do the delete test with standard Python List:: "
my_obj = []
i = 0
cur_time = datetime.datetime.now()
while i < 250000:
	my_obj.append("hello - this is a string")
	i += 1
while i > 0:
	del my_obj[0]
	i -= 1
print len(my_obj)
print datetime.datetime.now() - cur_time
print "Next try the linked list:: "
my_obj = LList(250000)
cur_time = datetime.datetime.now()
while i < 250000:
        my_obj.append("hello - this is a string")
        i += 1
while i > 0:
        del my_obj[0]
        i -= 1
print len(my_obj)
print datetime.datetime.now() - cur_time
print "Now do the concat test with a Python List:: "
my_obj = []
my_obj2 = [] 
i = 0
while i < 10000000:
        my_obj.append("hello - this is a string")
        i += 1
i = 0
while i < 10000000:
        my_obj2.append("this is a second string")
        i += 1
cur_time = datetime.datetime.now()
my_obj = my_obj + my_obj2
print len(my_obj)
print datetime.datetime.now() - cur_time
print "Now with a Linked List:: "
my_obj = LList(10000000) 
my_obj2 = LList(10000000)
i = 0
while i < 10000000:
        my_obj.append("hello - this is a string")
        i += 1
i = 0
while i < 10000000:
        my_obj2.append("this is a second string")
        i += 1
cur_time = datetime.datetime.now()
my_obj = my_obj + my_obj2
print len(my_obj)
print datetime.datetime.now() - cur_time
