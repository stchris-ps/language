int compstr(const char *c,const char *d);
char *conv2str(int x);
int convnum(char *a);
class exeption{
        int strnum;
        char *err;
        char *lex;
public:
        exeption(int i,const char *c,char *d);
        ~exeption();
        exeption(const exeption &t);
        int getnum();
        char *geterr();
        char *gettok();
};

char *fullstr(const char *c);

enum typelex{
        variable,
        delimiter,
        string,
        constant,
        label,
        key_word,
        def_function
};

enum stateid{H, I, S, K, N, A, C};

struct data4serv{
        int sellpric;
        int sellnum;
        int buypric;
        int buynum;
        int numfact;
        int makenum;
        int endturn;
};
struct vartable{
        char *name;
        long long value;
        vartable *next;
};
struct labtable{
        RPNItem *val;
        char *name;
        labtable *next;
};

struct infofromserv{
        int freeraw;
        int freeprod;
        int pricprod;
        int pricraw;
        int mynum;
        int myprod;
        int myraw;
        int mywealth;
        int myfact;
        int *enemyraw;
        int *enemyprod;
        int *enemypenny;
        int *enemyfact;
        int monthnum;
        int enemyarmy;
        int liveenemy;
        int *auctraw;
        int *auctprod;
        int *auctrawpr;
        int *auctprodpr;
        int iter;
        int enemynum;
        int result;
};

struct RPNItem;

