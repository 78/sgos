// system string

// 临时如此山寨一下，以后有空再抽象出一个子类。
// 按照ib所说：typedef std::basic_string <char, xxx> string;
class string
{
private:
	char *buf;
	int len;
	int buf_size;
	void init();
	void mksize( int siz );
	void copy( const char* str );
public:
	string();
	string( const char *src );
	//拷贝函数
	string( const string &src );
	~string();
	//操作符重载
	char& operator [] ( int index );
	string& operator = ( const char *src );
	string& operator = ( const string& s );
	bool operator == ( const char *src );
	bool operator == ( const string& s );
	string& operator + ( const string& add2 );
	string& operator + ( const char* src );
	string& operator += ( const char* src );
	string& operator += ( const string& add2 );
	operator char *();
	operator int();
	//成员函数
	const char *cstr();
	char charAt( int index );
	string& fromInteger( int number );
	int length();
	int find( string substr , int start=0 );
	int rfind( string substr );
	string trim( char part );
	string left( int sublen );
	string right( int sublen );
	string mid( int start , int sublen );
	string replace( string src , string dest );
};
