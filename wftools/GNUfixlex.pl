s/malloc\( size \);/new char[size];/;
s/return \(void \*\) realloc\( \(char \*\) ptr, size \);/assert\(0\); return 0;/;
s/free\( ptr \);/assert\(ptr\); delete [] ptr;/;
s/class istream;/#include <iostream>\nusing namespace std;/;
