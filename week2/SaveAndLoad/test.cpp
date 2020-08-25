#include "saveload.h"

#include "test_runner.h"

#include <map>
#include <sstream>
#include <string>

using namespace std;

void TestSaveLoad() {
    map<uint32_t, string> m = {
      {1, "hello"},
      {2, "bye"},
    };
    stringstream ss;
    Serialize(m, ss);
    auto s = ss.str();
    //cerr << sizeof(size_t) << endl;
    //cerr << s.size() << endl;
    //cerr << s << endl;
    ASSERT_EQUAL(s.size(), 40);

    // ������ map (8 ����, ��� ��� ��� size_t)
    ASSERT_EQUAL(s[0], 0x02);
    ASSERT_EQUAL(s[1], 0x00);
    ASSERT_EQUAL(s[2], 0x00);
    ASSERT_EQUAL(s[3], 0x00);
    ASSERT_EQUAL(s[4], 0x00);
    ASSERT_EQUAL(s[5], 0x00);
    ASSERT_EQUAL(s[6], 0x00);
    ASSERT_EQUAL(s[7], 0x00);

    // ���� 1 (4 �����, ��� ��� ��� uint32_t)
    ASSERT_EQUAL(s[8], 0x01);
    ASSERT_EQUAL(s[9], 0x00);
    ASSERT_EQUAL(s[10], 0x00);
    ASSERT_EQUAL(s[11], 0x00);

    // ��������: ������� ������ ������ (size_t)
    ASSERT_EQUAL(s[12], 0x05);
    ASSERT_EQUAL(s[13], 0x00);
    ASSERT_EQUAL(s[14], 0x00);
    ASSERT_EQUAL(s[15], 0x00);
    ASSERT_EQUAL(s[16], 0x00);
    ASSERT_EQUAL(s[17], 0x00);
    ASSERT_EQUAL(s[18], 0x00);
    ASSERT_EQUAL(s[19], 0x00);

    // ������ ���� ������ (ASCII-����):
    ASSERT_EQUAL(s[20], 0x68);  // 'h'
    ASSERT_EQUAL(s[21], 0x65);  // 'e'
    ASSERT_EQUAL(s[22], 0x6c);  // 'l'
    ASSERT_EQUAL(s[23], 0x6c);  // 'l'
    ASSERT_EQUAL(s[24], 0x6f);  // 'o'

    // ���� 2
    ASSERT_EQUAL(s[25], 0x02);
    ASSERT_EQUAL(s[26], 0x00);
    ASSERT_EQUAL(s[27], 0x00);
    ASSERT_EQUAL(s[28], 0x00);

    // �������� ����� 2: ������
    ASSERT_EQUAL(s[29], 0x03);
    ASSERT_EQUAL(s[30], 0x00);
    ASSERT_EQUAL(s[31], 0x00);
    ASSERT_EQUAL(s[32], 0x00);
    ASSERT_EQUAL(s[33], 0x00);
    ASSERT_EQUAL(s[34], 0x00);
    ASSERT_EQUAL(s[35], 0x00);
    ASSERT_EQUAL(s[36], 0x00);

    // �������� ����� 2: ������
    ASSERT_EQUAL(s[37], 0x62);  // 'b'
    ASSERT_EQUAL(s[38], 0x79);  // 'y'
    ASSERT_EQUAL(s[39], 0x65);  // 'e'

    map<uint32_t, string> m2;
    Deserialize(ss, m2);

    ASSERT_EQUAL(m, m2);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSaveLoad);
    return 0;
}
