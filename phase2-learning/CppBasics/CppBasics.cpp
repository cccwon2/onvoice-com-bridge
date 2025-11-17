#include <iostream>
using namespace std;

int main()
{
    cout << "=== 포인터 실습 ===" << endl << endl;

    // 1. 일반 변수
    int age = 42;
    cout << "age 값: " << age << endl;
    cout << "age 메모리 주소: " << &age << endl << endl;

    // 2. 포인터 변수 (메모리 주소를 담는 변수)
    int* ptr = &age;  // age의 주소를 ptr에 저장
    cout << "ptr이 가리키는 주소: " << ptr << endl;
    cout << "ptr이 가리키는 값 (*ptr): " << *ptr << endl << endl;

    // 3. 포인터로 값 변경
    *ptr = 100;  // ptr이 가리키는 곳(age)의 값을 100으로 변경
    cout << "포인터로 변경 후 age: " << age << endl << endl;

    // 4. NULL 포인터 (중요!)
    int* nullPtr = NULL;  // 또는 nullptr (C++11 이상)
    if (nullPtr == NULL) {
        cout << "nullPtr은 아무것도 가리키지 않음" << endl;
        // *nullPtr = 10;  // ❌ 크래시 발생! 절대 하면 안 됨
    }

    cout << endl << "=== 참조 실습 ===" << endl << endl;

    // 5. 참조 (Reference): 변수의 별명
    int original = 50;
    int& ref = original;  // ref는 original의 별명

    cout << "original: " << original << endl;
    cout << "ref: " << ref << endl << endl;

    ref = 200;  // ref를 바꾸면 original도 바뀜
    cout << "ref 변경 후 original: " << original << endl;

    return 0;
}