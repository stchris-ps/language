#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<string.h>
#include "simp_function.h"

enum{codeslash=92};
enum {lendig=10};
enum {length=4096};
enum {str4read=4};
enum {str5read=5};
enum {makeprice=2000};
#define ANALYS_MARKET 0
#define ANALYS_YOURSELF 1
#define ANALYS_AUCTION 2
#define ANALYS_ENEMYSELF 3

enum typelex{
	variable,
	delimiter,
	string,
	constant,
	label,
	key_word,
	def_function
};

enum stateid{H,	I, S, K, N, A, C};

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

struct RPNItem;

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

int compstr(const char *c,const char *d)
{
	int i;
	i=0;
	while(c[i]&&d[i]&&(c[i]==d[i])){
		i++;
	}
	return c[i]==d[i];
}

char *conv2str(int x)
{
	int y=x;
	int i=0;
	char *c;
	while(y)
	{
		i++;
		y=y/10;
	}
	c=new char[i+1];
	c[i]=0;
	i--;
	while(x)
	{
		c[i]='0'+x%10;
		i--;
		x=x/10;
	}
	return c;
}


class RPNElem{
public:
	virtual void Evaluate(RPNItem **st, RPNItem **cur) = 0;
	virtual ~RPNElem() {}
	static void push(RPNItem **st, RPNElem *elem);
	static RPNElem *pop(RPNItem **st);
	static RPNItem *lastel(RPNItem *rpn);
};

struct RPNItem{
	RPNElem *elem;
	RPNItem *next;
};

RPNItem* RPNElem::lastel(RPNItem *rpn)
{
	RPNItem *R;
	R=rpn;
	while(R->next)
		R=R->next;
	return R;
}

void RPNElem::push(RPNItem **st, RPNElem *elem)
{
	RPNItem *p;
	RPNItem *q;
	q=new RPNItem;
	q->elem=elem;
	q->next=NULL;
	if(!(*st)){
		(*st)=q;
	}else{
		p=*st;
		while(p->next)
		{
			p=p->next;
		}
		p->next=q;
	}

}

RPNElem* RPNElem::pop(RPNItem **st)
{
	RPNItem *p;
	RPNElem *q;
	if(!(*st)->next){
		q=(*st)->elem;
		*st=NULL;
	}else{
		p=*st;
		while(p->next->next)
		{
			p=p->next;
		}
		q=p->next->elem;
		p->next=NULL;
	}
	return q;
}

class open_br : public RPNElem{
public:
	open_br() {}
	virtual ~open_br() {}
	virtual void Evaluate(RPNItem **st, RPNItem **cur) {}
	RPNElem *clone() 
	{ return new open_br(); }
};

class close_br : public RPNElem{
public:
	close_br() {}
	virtual ~close_br() {}
	virtual void Evaluate(RPNItem **st, RPNItem **cur) {}
	RPNElem *clone() 
	{ return new close_br(); }
};


class RPNEmpty : public RPNElem{
public:
	RPNEmpty() {}
	virtual void Evaluate(RPNItem **st, RPNItem **cur)
       	{
		*cur=(*cur)->next;
	}
	RPNElem *clone() 
	{ return new RPNEmpty(); }
};

class RPNConst : public RPNElem{
public:
	virtual RPNElem* clone() = 0;
	virtual void Evaluate(RPNItem **st, RPNItem **cur) 
	{
		push(st,clone());
		*cur=(*cur)->next;
	}
};

class RPNvar : public RPNElem{
	char *name;
public:
	RPNvar(char *c)	{ name=c; }
	virtual RPNElem* clone() 
	{ return new RPNvar(name); }
	virtual void Evaluate(RPNItem **st, RPNItem **cur) 
	{
		push(st,clone());
		*cur=(*cur)->next;
	}
	char* get() { return name; }
};

class RPNFunction : public RPNElem{
public:
	virtual RPNElem *Evaluatefun(RPNItem **st) = 0;
	virtual void Evaluate(RPNItem **st, RPNItem **cur) 
	{
		RPNElem *x=Evaluatefun(st);
		if(x)
			push(st,x);
		*cur=(*cur)->next;
	}
};

class RPNnum : public RPNConst{
	long long val;
public:
	RPNnum(long long x) { val=x; }
	virtual ~RPNnum() {}
	virtual RPNElem *clone() 
	{ return new RPNnum(val); }
	long long get() { return val; }
};

class RPNlab : public RPNConst{
	RPNItem *val;
public:
	RPNlab(RPNItem *x) { val=x; }
	RPNlab() { val=NULL; }
	void full(RPNItem *x) { val=x; }
	virtual ~RPNlab() {}
	RPNItem *get() { return val; }
	virtual RPNElem *clone() 
	{ return new RPNlab(val); }
};

class RPNstr : public RPNConst{
	char *val;
public:
	RPNstr(char *x) { val=x; }
	virtual ~RPNstr() {}
	virtual RPNElem *clone() 
	{ return new RPNstr(val); }
	char* get() { return val; }
};

class RPNvarad : public RPNConst{
	long long *val;
public:
	RPNvarad(long long x) { val=&x; }
	virtual ~RPNvarad() {}
	virtual RPNElem *clone()
	{ return new RPNvarad(*val); }
	long long *get() { return val; }
};

class RPNnonjump : public RPNElem{
public:
	RPNnonjump() {}
	virtual ~RPNnonjump() {}
	virtual void Evaluate(RPNItem **st, RPNItem **cur)
	{
		RPNElem *op1=pop(st);
		RPNlab *lab=dynamic_cast<RPNlab*>(op1);
		if(!lab) throw "exist 1 wrong label";
		RPNElem *op2=pop(st);
		RPNnum *cond=dynamic_cast<RPNnum*>(op2);
		if(!cond) throw "exist 1 wrong condition";
		if(!cond->get()){
			RPNItem *addr=lab->get();
			*cur=addr;
		}else{
			*cur=(*cur)->next;
		}
		delete op1;
		delete op2;
	}
	RPNElem *clone()
	{ return new RPNnonjump(); }
};


class RPNjump : public RPNElem{
public:
	RPNjump() {}
	virtual ~RPNjump() {}
	virtual void Evaluate(RPNItem **st, RPNItem **cur)
	{
		RPNElem *op1=pop(st);
		RPNlab *lab=dynamic_cast<RPNlab*>(op1);
		if(!lab) throw "exist 1 wrong label";
		RPNItem *addr=lab->get();
		*cur=addr;
		delete op1;
	}
	RPNElem *clone()
	{ return new RPNjump(); }
};

class RPNor : public RPNFunction{
public:
	RPNor() {}
	virtual ~RPNor() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in addition";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in addition";
		int res=l1->get()||l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNor(); }
};

class RPNand : public RPNFunction{
public:
	RPNand() {}
	virtual ~RPNand() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in addition";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in addition";
		long long res=l1->get()&&l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone() 
	{ return new RPNand(); }
};

class RPNplus : public RPNFunction{
public:
	RPNplus() {}
	virtual ~RPNplus() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in addition";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in addition";
		long long res=l1->get()+l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone() 
	{ return new RPNplus(); }
};

class RPNminus : public RPNFunction{
public:
	RPNminus() {}
	virtual ~RPNminus() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in substraction";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in substraction";
		long long res=l2->get()-l1->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNminus(); }
};

class RPNunmin : public RPNFunction{
public:
	RPNunmin() {}
	virtual ~RPNunmin() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in unary minus";
		long long res=-l1->get();
		delete op1;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNunmin(); }
};

class RPNunnot : public RPNFunction{
public:
	RPNunnot() {}
	virtual ~RPNunnot() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in unary not";
		long res=!l1->get();
		delete op1;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNunnot(); }
};


class RPNmul : public RPNFunction{
public:
	RPNmul() {}
	virtual ~RPNmul() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in multiplication";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in multiplication";
		long long res=l1->get()*l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNmul(); }
};

class RPNdiv : public RPNFunction{
public:
	RPNdiv() {}
	virtual ~RPNdiv() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in division";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in division";
		if(l2->get()==0) throw "division by zero";
		long long res=l2->get()/l1->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNdiv(); }
};

class RPNbitand : public RPNFunction{
public:
	RPNbitand() {}
	virtual ~RPNbitand() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in 'and'";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in 'and'";
		long long res=l1->get()&l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNbitand(); }
};

class RPNbitor : public RPNFunction{
public:
	RPNbitor() {}
	virtual ~RPNbitor() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in 'or'";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in 'or'";
		long long res=l1->get()|l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNbitor(); }
};

class RPNindex : public RPNFunction{
	vartable *t;
public:
	RPNindex(vartable *tabl) { t=tabl; }
	virtual ~RPNindex() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in indexing";
		RPNElem *op1=pop(st);
		RPNvar *l1=dynamic_cast<RPNvar*>(op1);
		if(!l1) throw "exist wrong parametr in indexing";
		char *name=strcat(l1->get(),conv2str(l2->get()));
		vartable *h=t;
		vartable *h1,*h2;
		while(h)
		{
			if(compstr(h->name,name))
				break;
			h1=h;
			h=h->next;
		}
		if(!h){
			h2=new vartable;
			h2->name=name;
			h2->value=l2->get();
			h2->next=NULL;
			h1->next=h2;
		}
		delete op1;
		delete op2;
		return new RPNvar(name);
	}
	RPNElem *clone() 
	{ return new RPNindex(t); }
};

class RPNlarger : public RPNFunction{
public:
	RPNlarger() {}
	virtual ~RPNlarger() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in comparison(>)";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in comparison(>)";
		int res=l1->get()<l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNlarger(); }
};

class RPNeq : public RPNFunction{
public:
	RPNeq() {}
	virtual ~RPNeq() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in comparison(=)";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in comparison(=)";
		int res=l1->get()==l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNeq(); }
};

class RPNlargereq : public RPNFunction{
public:
	RPNlargereq() {}
	virtual ~RPNlargereq() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in comparison(>=)";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in comparison(>=)";
		int res=l1->get()<=l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNlargereq(); }
};

class RPNless : public RPNFunction{
public:
	RPNless() {}
	virtual ~RPNless() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in comparison(<)";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in comparison(<)";
		int res=(l1->get()>l2->get());
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNless(); }
};

class RPNlesseq : public RPNFunction{
public:
	RPNlesseq() {}
	virtual ~RPNlesseq() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in comparison(<=)";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in comparison(<=)";
		int res=l1->get()>=l2->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNlesseq(); }
};

class RPNassign : public RPNFunction{
	vartable* table;
public:
	RPNassign(vartable* t) { table=t; }
	virtual ~RPNassign() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		vartable* t=table;
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in assignment";
		RPNElem *op1=pop(st);
		RPNvar *l1=dynamic_cast<RPNvar*>(op1);
		if(!l1) throw "exist wrong parametr in assignment";
		while(t)
		{
			if(compstr(t->name,l1->get())){
				t->value=l2->get();
				break;
			}
			t=t->next;
		}
		if(!t) throw "this variable is not exist";
		delete op1;
		delete op2;
		return NULL;
	}
	RPNElem *clone()
	{ return new RPNassign(table); }
};

class RPNmod : public RPNFunction{
public:
	RPNmod() {}
	virtual ~RPNmod() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr in mod";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr in mod";
		long long res=l2->get()%l1->get();
		delete op1;
		delete op2;
		return new RPNnum(res);
	}
	RPNElem *clone()
	{ return new RPNmod(); }
};

class RPNsell : public RPNFunction{
	data4serv *data;
public:
	RPNsell(data4serv *d) { data=d; }
	virtual ~RPNsell() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for sell";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr for sell";
		data->sellnum=l2->get();
		data->sellpric=l1->get();
		delete op1;
		delete op2;
		return NULL;
	}
	RPNElem *clone()
	{ return new RPNsell(data); }
};

class RPNbuy : public RPNFunction{
	data4serv *data;
public:
	RPNbuy(data4serv *d) { data=d; }
	virtual ~RPNbuy() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for buy";
		RPNElem *op2=pop(st);
		RPNnum *l2=dynamic_cast<RPNnum*>(op2);
		if(!l2) throw "exist wrong parametr for buy";
		data->buynum=l2->get();
		data->buypric=l1->get();
		delete op1;
		delete op2;
		return NULL;
	}
	RPNElem *clone()
	{ return new RPNbuy(data); }
};

class RPNmake : public RPNFunction{
	data4serv *data;
public:
	RPNmake(data4serv *d) { data=d; }
	virtual ~RPNmake() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for make";
		data->makenum=l1->get();
		delete op1;
		return NULL;
	}
	RPNElem *clone()
	{ return new RPNmake(data); }
};

class RPNbuild : public RPNFunction{
	data4serv *data;
public:
	RPNbuild(data4serv *d) { data=d; }
	virtual ~RPNbuild() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		data->numfact++;
		return NULL;
	}
	RPNElem *clone()
	{ return new RPNbuild(data); }
};

class RPNstf : public RPNFunction{
	data4serv *data;
public:
	RPNstf(data4serv *d) { data=d; }
	virtual ~RPNstf() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		data->endturn=1;
		return NULL;
	}
	RPNElem *clone() 
	{ return new RPNstf(data); }
};

class RPNfreeraw : public RPNFunction{
	infofromserv *info;
public:
	RPNfreeraw(infofromserv *i) { info=i; }
	virtual ~RPNfreeraw() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		return new RPNnum(info->freeraw);
	}
};

class RPNfreeprod : public RPNFunction{
	infofromserv *info;
public:
	RPNfreeprod(infofromserv *i) { info=i; }
	virtual ~RPNfreeprod() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->freeprod);
	}
};

class RPNpricprod : public RPNFunction{
	infofromserv *info;
public:
	RPNpricprod(infofromserv *i) { info=i; }
	virtual ~RPNpricprod() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->pricprod);
	}
};

class RPNpricraw : public RPNFunction{
	infofromserv *info;
public:
	RPNpricraw(infofromserv *i) { info=i; }
	virtual ~RPNpricraw() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->pricraw);
	}
};

class RPNmynum : public RPNFunction{
	infofromserv *info;
public:
	RPNmynum(infofromserv *i) { info=i; }
	virtual ~RPNmynum() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->mynum);
	}
};

class RPNmyprod : public RPNFunction{
	infofromserv *info;
public:
	RPNmyprod(infofromserv *i) { info=i; }
	virtual ~RPNmyprod() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->myprod);
	}
};

class RPNmyraw : public RPNFunction{
	infofromserv *info;
public:
	RPNmyraw(infofromserv *i) { info=i; }
	virtual ~RPNmyraw() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->myraw);
	}
};

class RPNmywealth : public RPNFunction{
	infofromserv *info;
public:
	RPNmywealth(infofromserv *i) { info=i; }
	virtual ~RPNmywealth() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->mywealth);
	}
};

class RPNmyfact : public RPNFunction{
	infofromserv *info;
public:
	RPNmyfact(infofromserv *i) { info=i; }
	virtual ~RPNmyfact() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->myfact);
	}
};

class RPNenemyraw : public RPNFunction{
	infofromserv *info;
public:
	RPNenemyraw(infofromserv *i) { info=i; }
	virtual ~RPNenemyraw() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for enemyraw";
		return new RPNnum(info->enemyraw[l1->get()]);
	}
};

class RPNenemyprod : public RPNFunction{
	infofromserv *info;
public:
	RPNenemyprod(infofromserv *i) { info=i; }
	virtual ~RPNenemyprod() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for enemyprod";
		return new RPNnum(info->enemyprod[l1->get()]);
	}
};

class RPNenemypenny : public RPNFunction{
	infofromserv *info;
public:
	RPNenemypenny(infofromserv *i) { info=i; }
	virtual ~RPNenemypenny() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for enemypenny";
		return new RPNnum(info->enemypenny[l1->get()]);
	}
};

class RPNmonthnum : public RPNFunction{
	infofromserv *info;
public:
	RPNmonthnum(infofromserv *i) { info=i; }
	virtual ~RPNmonthnum() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->monthnum);
	}
};

class RPNenemyarmy : public RPNFunction{
	infofromserv *info;
public:
	RPNenemyarmy(infofromserv *i) { info=i; }
	virtual ~RPNenemyarmy() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->enemyarmy);
	}
};

class RPNliveenemy : public RPNFunction{
	infofromserv *info;
public:
	RPNliveenemy(infofromserv *i) { info=i; }
	virtual ~RPNliveenemy() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		return new RPNnum(info->liveenemy);
	}
};

class RPNauctraw : public RPNFunction{
	infofromserv *info;
public:
	RPNauctraw(infofromserv *i) { info=i; }
	virtual ~RPNauctraw() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for make";
		return new RPNnum(info->auctraw[l1->get()]);
	}
};

class RPNauctprod : public RPNFunction{
	infofromserv *info;
public:
	RPNauctprod(infofromserv *i) { info=i; }
	virtual ~RPNauctprod() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for make";
		if(info->monthnum==0) throw "auction was not holded";
		return new RPNnum(info->auctprod[l1->get()]);
	}
};

class RPNauctrawpr : public RPNFunction{
	infofromserv *info;
public:
	RPNauctrawpr(infofromserv *i) { info=i; }
	virtual ~RPNauctrawpr() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for make";
		if(info->monthnum==0) throw "auction was not holded";
		return new RPNnum(info->auctrawpr[l1->get()]);
	}
};

class RPNauctprodpr : public RPNFunction{
	infofromserv *info;
public:
	RPNauctprodpr(infofromserv *i) { info=i; }
	virtual ~RPNauctprodpr() {}
	RPNElem *Evaluatefun(RPNItem **st)
	{
		RPNElem *op1=pop(st);
		RPNnum *l1=dynamic_cast<RPNnum*>(op1);
		if(!l1) throw "exist wrong parametr for make";
		if(info->monthnum==0) throw "auction was not holded";
		return new RPNnum(info->auctprodpr[l1->get()]);
	}
};

class RPNval : public RPNFunction{
	vartable *table;
public:
	RPNval(vartable *t) { table=t; }
	virtual ~RPNval() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNvar *l1=dynamic_cast<RPNvar*>(op1);
		if(!l1) throw "error in taking valuate";
		vartable *v=table;
		while(v)
		{
			if(compstr(l1->get(),v->name)){
				return new RPNnum(v->value);
			}
			v=v->next;
		}
		throw "this variable is not exist";
	}
	RPNElem *clone()
	{ return new RPNval(table); }
};

class RPNprint : public RPNFunction{
public:
	RPNprint() {}
	virtual ~RPNprint() {}
	RPNElem *Evaluatefun(RPNItem **st) 
	{
		RPNElem *op1=pop(st);
		RPNstr *l1=dynamic_cast<RPNstr*>(op1);
		if(l1){
			printf("%s\n",l1->get());
		}else{
			RPNnum *l2=dynamic_cast<RPNnum*>(op1);
			if(!l2) throw "wrong paarametr for print";
			printf("%lli\n",l2->get());
		}
		return NULL;
	}
	RPNElem *clone()
	{ return new RPNprint(); }
};


class token{
	typelex type;
	char *cont;
	int number;
public:
	token()
	{ cont=NULL; }
	token(const token&);
	~token()
	{delete[] cont;}
	void joinb(char *c)
	{ cont=c; }
	void givenum(int i)
	{ number=i; }
	void givetype(typelex y)
	{ type=y; }
	int getnum()
	{ return number; }
	char *getcont()
	{ return cont; }
	typelex gettype()
	{ return type; }
	void makenorm();
};

void token::makenorm()
{
	char *c;
	int i,j;
	for(i=0;cont[i];i++)
	{}
	c=new char[i];
	for(j=0;cont[j+1]!='"';j++)
		c[j]=cont[j+1];
	c[j]=0;
	delete[] cont;
	cont=c;
}

class machine
{
	stateid state;
	char *contb;
	int quot,strnum,flagnewstr,brflag;
	int c;
public:
	machine()
	{
		state=H;
		quot=0;
		c=0;
		flagnewstr=0;
		strnum=0;
		contb=new char[1024];
		contb[0]=0;
		brflag=0;
	}
	machine(const machine&);
	~machine()
	{	
		delete[] contb;
	}
	bool feed(int&);
	char* getbuf();
	int getquot() {return quot;}
	int getflagn() {return flagnewstr;}
	int getstr() {return strnum;}
	void cutbuf() {contb[0]=0;}
private:
	bool step(int&);
	bool caseN(int&);
	bool caseA(int&);
	bool caseH(int&);
	bool caseS(int&);
	bool caseI(int&);
	bool caseK(int&);
	bool caseC(int&);
};

int strleng(const char *c)
{
	int i;
	for(i=0;c[i];i++)
	{}
	return i;
}

machine::machine(const machine &c)
{
	int j;
	j=strleng(c.contb);
	contb=new char[j+2];
	for(int i=0;i<j;i++)
		contb[i]=c.contb[i];
}

char* machine::getbuf()
{
	int i;
	char *p;
	i=strleng(contb);
	p=new char[i+1];
	for(int j=0;j<=i;j++)
		p[j]=contb[j];
	return p;
}

token::token(const token& t)
{
	int j;
	j=strleng(t.cont);
	cont=new char[j+2];
	for(int i=0;i<j;i++)
		cont[i]=t.cont[i];
}

#if 0
char *masup10(char *h,int i)//перенеси в методы
{
	char *q;
	const char sizeup=11;
	int j;
	q=NULL;
	q=new char[i+sizeup];
	for(j=0;j<i;j++)
		q[j]=h[j];
	delete[] h;
	return q;
}
#endif

int compar1(char g)
{
	if((g=='+')||(g=='-')||(g=='/')||(g=='%')||(g=='*')||(g==' '))
		return 1;
	if((g=='<')||(g=='>')||(g=='=')||(g==':')||(g==';')||(g==','))
		return 1;
	if((g==')')||(g=='(')||(g=='[')||(g==']')||(g=='{')||(g=='}'))
		return 1;
	return 0;
}

int compar2(char g)
{
	if((g=='`')||((g>='A')&&(g<='Z'))||((g>='a')&&(g<='z')))
		return 1;
	if(((g>='0')&&(g<='9'))||(g=='_')||(g=='^')||(g==codeslash))
		return 1;
	return 0;
}

int compar3(char g)
{
	if(((g>='A')&&(g<='Z'))||((g>='a')&&(g<='z')))
		return 1;
	return 0;
}

bool machine::caseN(int &g){
	int j;
	state=N;
	if(compar1(g)){
		state=H;
		c=g;
		return 1;
	}
	if((g>='0')&&(g<='9')){
		j=strleng(contb);
		contb[j]=g;
		contb[j+1]=0;
		return 0;
	}
	state=H;
	printf("invalid number format\n");
	exit(1);
	return 1;
}

bool machine::caseI(int &g){
	int j;
	state=I;
	if(compar2(g)){
		j=strleng(contb);
		contb[j]=g;
		contb[j+1]=0;
		return 0;
	}
	if(compar1(g)){
		state=H;
		c=g;
		return 1;
	}
	state=H;
	printf("invalid identifire format\n");
	exit(1);
	return 1;
}

bool machine::caseK(int &g){
	int j;
	state=K;
	if(compar3(g)){
		j=strleng(contb);
		contb[j]=g;
		contb[j+1]=0;
		return 0;
	}
	if(compar1(g)){
		state=H;
		c=g;
		return 1;
	}
	state=H;
	printf("invalid key word\n");
	exit(1);
	return 1;
}

bool machine::caseS(int &g){
	int j;
	state=S;
	j=strleng(contb);
	contb[j]=g;
	contb[j+1]=0;
	if((g=='"')&&(quot)){
		state=H;
		quot=1-quot;
		return 1;
	}
	quot=1;
	return 0;
}

bool machine::caseH(int &g){
	if((g=='`')||(g==codeslash)||(g=='^')){
		caseI(g);
		return 0;
	}
	if((g<='9')&&(g>='0')){
		caseN(g);
		return 0;
	}
	if(g=='"'){
		caseS(g);
		return 0;
	}
	if(g==':'){
		caseA(g);
		return 0;
	}
	if(((g>='a')&&(g<='z'))||((g>='A')&&(g<='Z'))){
		caseK(g);
		return 0;
	}
	if((g=='<')||(g=='>')){
		caseC(g);
		return 0;
	}
	if(compar1(g)){
		contb[1]=0;
		contb[0]=g;
		return 1;
	}
	return 0;
}

bool machine::caseA(int &g){
	int j;
	state=A;
	if((g==':')||(g=='=')){
		j=strleng(contb);
		contb[j]=g;
		contb[j+1]=0;
		if(g=='='){
			state=H;
			return 1;
		}
		return 0;
	}
	c=g;
	state=H;
	return 1;
}

bool machine::caseC(int &g){
	state=C;
	if((g=='<')||(g=='>')){
		contb[0]=g;
		contb[1]=0;
		return 0;
	}
	if(g=='='){
		contb[1]=g;
		contb[2]=0;
		state=H;
		return 1;
	}
	c=g;
	state=H;
	return 1;
}
	
bool machine::step(int &g)
{
	strnum+=flagnewstr;
	flagnewstr=0;
	if(((g==' ')||(g=='\n'))&&(!quot)){
		if(g=='\n')
			flagnewstr=1;
		if(state==H)
			return 0;
		state=H;
		return 1;
	}
	if(state==N)
		return caseN(g);
	if(state==I)
		return caseI(g);
	if(state==K)
		return caseK(g);
	if(state==S)
		return caseS(g);
	if(state==H)
		return caseH(g);
	if(state==A)
		return caseA(g);
	if(state==C)
		return caseC(g);
	return 0;
}

bool machine::feed(int &g)
{
	bool b;
	if(c){
		b=step(c);
		c=0;
		if(!b){
			b=step(g);
		}else{
			c=g;
		}
	}else{
		b=step(g);
	}
	return b;
}

token *moretok(token *k,int i)
{
	token *p;
	p=new token[i+10];
	for(int j=0;j<i;j++)
		p[j]=k[j];
	return p;
}

typelex findol(char *c)
{
	if(compar1(c[0]))
		return delimiter;
	if((c[0]<='9')&&(c[0]>='0'))
		return constant;
	if(c[0]=='"')
		return string;
	if(c[0]==codeslash)
		return variable;
	if(c[0]=='^')
		return label;
	if(c[0]=='`')
		return def_function;
	return key_word;
}

char* makestr(typelex type)
{
	switch(type){
		case variable:
			return fullstr("variable");
		case delimiter:
			return fullstr("delimiter");
		case string:
			return fullstr("string");
		case constant:
			return fullstr("constant");
		case label:
			return fullstr("label");
		case key_word:
			return fullstr("key_word");
		case def_function:
			return fullstr("def_function");
	}
	return NULL;
}

void outmas(token *t,int len)
{
	int i;
	char *p,*q;
	for(i=0;i<len;i++)
	{
		p=t[i].getcont();
		q=makestr(t[i].gettype());
		printf("%s  |%s  |%d\n",p,q,t[i].getnum());
		delete[] q;
	}
}

void testquot(machine eng)
{
	if(eng.getflagn())
		if(eng.getquot())
			fprintf(stderr,"quotation marks disbalanced\n");
}

void testargc(int h)
{
	if(h<2){
		printf("give me a file\n");
		exit(1);
	}
}

void testfilvar(FILE *f,char *c)
{
	if(!f){
		perror(c);
		exit(1);
	}
}

void senddatatoserv(data4serv &d,int sd)
{
	if(d.buynum){
		dprintf(sd,"buy %d/%d\n",d.buynum,d.buypric);
		d.buynum=0;
		d.buypric=0;
	}
	if(d.makenum){
		dprintf(sd,"make %d\n",d.makenum);
		d.makenum=0;
	}
	if(d.sellnum){
		dprintf(sd,"sell %d/%d\n",d.sellnum,d.sellpric);
		d.sellnum=0;
		d.sellpric=0;
	}
	while(d.numfact){
		dprintf(sd,"build\n");
		d.numfact--;
	}
	dprintf(sd,"stf\n");
}

void initinfo(infofromserv &i)
{
	i.freeraw=0;
	i.freeprod=0;
	i.pricprod=0;
	i.pricraw=0;
	i.mynum=0;
	i.myprod=0;
	i.myraw=0;
	i.mywealth=0;
	i.myfact=0;
	i.enemyraw=NULL;
	i.enemyprod=NULL;
	i.enemypenny=NULL;
	i.monthnum=0;
	i.enemyarmy=0;
	i.liveenemy=0;
	i.auctraw=NULL;
	i.auctprod=NULL;
	i.auctrawpr=NULL;
	i.auctprodpr=NULL;
}

char *rdinbuf(char *usbuf,int &fd)
{
        int i,ad;
        for(i=0;usbuf[i];i++)
        {}
        ad=read(fd,&usbuf[i],length-i);
        if(!ad){
                shutdown(fd,SHUT_RDWR);
                close(fd);
                fd=0;
        }
        i+=ad;
        usbuf[i]='\0';
        return usbuf;
}

bool testent(char *c)
{
        int i;
        for(i=0;c[i];i++)
        {
                if(c[i]=='\n')
                        return 1;
        }
        return 0;
}

char *cutbuf(char *usbuf,int j)
{
        int i;
        char *c;
        c=new char[length];
        for(i=0;usbuf[i+j+1];i++)
                c[i]=usbuf[i+j+1];
        c[i]=0;
        delete[] usbuf;
        return c;
}

int findnzer(char *com)
{
        int i;
        for(i=0;com[i]==' ';i++)
        {}
        return i;
}

int getlennum(int x)
{
        int i=0;
        int j=1;
        while((x)||(j))
        {
                x/=10;
                i++;
                j=0;
        }
        return i;
}

int makedig(char *com)
{
	int i=0;
	char str[lendig];
	while((com[i]!=' ')&&(com[i]))
	{
		str[i]=com[i];
		i++;
	}
	str[i]=0;
	return convnum(str);
}

infofromserv analysbankdata(char *com, infofromserv p)
{
        int i,j;
        i=findnzer(com);
        switch(p.iter){
                case 0:
                        p.monthnum=makedig(&com[i]);
                        p.iter++;
                        break;
                case 1:
                        p.freeraw=makedig(&com[i]);
                        j=findnzer(&com[i+getlennum(p.freeraw)]);
                        j+=getlennum(p.freeraw);
                        p.pricraw=makedig(&com[i+j]);
                        p.iter++;
                        break;
                case 2:
                        p.freeprod=makedig(&com[i]);
                        j=findnzer(&com[i+getlennum(p.freeprod)]);
                        j+=getlennum(p.freeprod);
                        p.pricprod=makedig(&com[i+j]);
                        p.iter++;
                        break;
                case 3:
                        p.enemyarmy=makedig(&com[i]);
                        p.iter++;
                        break;
        }
        return p;
}

infofromserv analysmydata(char *com, infofromserv p)
{
        int i;
        i=findnzer(com);
        switch(p.iter){
                case 0:
                        p.mynum=makedig(&com[i]);
                        p.iter++;
                        break;
                case 1:
                        p.myprod=makedig(&com[i]);
                        p.iter++;
                        break;
                case 2:
                        p.myraw=makedig(&com[i]);
                        p.iter++;
                        break;
                case 3:
                        p.myfact=makedig(&com[i]);
                        p.iter++;
                        break;
                case 4:
                        p.mywealth=makedig(&com[i]);
                        p.iter++;
                        break;
                break;
        }
        return p;
}

infofromserv analysenemydata(char *com, infofromserv p)
{
        int i;
        i=findnzer(com);
        switch(p.iter){
                case 0:
                        p.iter++;
                        break;
                case 1:
                        p.enemyprod[p.enemynum]=makedig(&com[i]);
                        p.iter++;
                        break;
                case 2:
                        p.enemyraw[p.enemynum]=makedig(&com[i]);
                        p.iter++;
                        break;
                case 3:
                        p.enemyfact[p.enemynum]=makedig(&com[i]);
                        p.iter++;
                        break;
                case 4:
                        p.enemypenny[p.enemynum]=makedig(&com[i]);
                        p.iter++;
                        break;
                break;
        }
        return p;
}

infofromserv analysauctiondata(char *com, infofromserv p)
{
        int i,num,j;
        i=findnzer(com);
	num=makedig(&com[i]);
	j=findnzer(&com[i+getlennum(num)]);
	j+=(getlennum(num)+1);
	p.auctraw[num]=makedig(&com[i+j]);
	i+=j;
	j=findnzer(&com[i+getlennum(p.auctraw[num])]);
	j+=(getlennum(p.auctraw[num])+1);
	p.auctrawpr[num]=makedig(&com[i+j]);
        return p;
}

infofromserv analysauctselldata(char *com, infofromserv p)
{
        int i,num,j;
        i=findnzer(com);
	num=makedig(&com[i]);
	j=findnzer(&com[i+getlennum(num)]);
	j+=(getlennum(num)+1);
	p.auctprod[num]=makedig(&com[i+j]);
	i+=j;
	j=findnzer(&com[i+getlennum(p.auctraw[num])]);
	j+=(getlennum(p.auctraw[num])+1);
	p.auctprodpr[num]=makedig(&com[i+j]);
        return p;
}

bool func4anrd(char *command)
{
	const char *s4="auction ended",*s1="the play was not start";
	const char *s2="you went bankrupt",*s3="Congratulations! You won!";
	if(compstr(command,s1)||compstr(command,s2))
		return 1;
	if(compstr(command,s3)||compstr(command,s4))
		return 1;
	return 0;
}

char *anrd(char *usbuf, infofromserv &p, int analys_flag)
{
        int i,j;
        char command[length];
        j=-1;
        for(i=0;usbuf[i];i++)
        {
                if(usbuf[i]=='\n'){
                        j=i;
                        break;
                }
        }
        i=0;
        if(j!=-1){
                for(i=0;i<j;i++)
                        command[i]=usbuf[i];
                command[j]=0;
                if((usbuf[0]!='%')&&(usbuf[0]!='$')){
                        if(func4anrd(command)){
                                usbuf[j]=0;
                        }else{
                                usbuf=cutbuf(usbuf,j);
                        }
                        return usbuf;
                }
		if(usbuf[0]=='%'){
	                if(analys_flag==ANALYS_MARKET)
        	                p=analysbankdata(&command[1],p);
                	if(analys_flag==ANALYS_YOURSELF)
                        	p=analysmydata(&command[1],p);
	                if(analys_flag==ANALYS_AUCTION)
        	                p=analysauctiondata(&command[1],p);
	                if(analys_flag==ANALYS_ENEMYSELF)
        	                p=analysenemydata(&command[1],p);
		}
		if(usbuf[0]=='$')
			p=analysauctselldata(&command[1],p);
                usbuf=cutbuf(usbuf,j);
        }
        return usbuf;
}

infofromserv analysauction(int sd,infofromserv info)
{
        char *bufer;
	const char *s1="auction ended";
	const char *s2="you went bankrupt";
	const char *s3="Congratulations! You won!";
	bufer= new char[length];
        bufer[0]=0;
	while(!(compstr(bufer,s1)||compstr(bufer,s2)||compstr(bufer,s3)))
	{
                if(!(testent(bufer)))
                        bufer=rdinbuf(bufer,sd);
                bufer=anrd(bufer,info,ANALYS_AUCTION);
                if(compstr(bufer,"the play was not start")){
                        sleep(1);
                        dprintf(sd,"marketsituation\n");
                        bufer[0]=0;
        	}
		if(compstr(bufer,s2))
			info.result=1;
		if(compstr(bufer,s3))
			info.result=2;
	}
        delete[] bufer;
        return info;
}

infofromserv findoutmarinfo(int sd,infofromserv info)
{
        char *bufer;
	bufer= new char[length];
        bufer[0]=0;
        dprintf(sd,"marketsituation\n");
        info.iter=0;
        while(info.iter<str4read)
        {
                if(!(testent(bufer)))
                        bufer=rdinbuf(bufer,sd);
                bufer=anrd(bufer,info,ANALYS_MARKET);
                if(compstr(bufer,"the play was not start")){
                        sleep(1);
                        dprintf(sd,"marketsituation\n");
                        bufer[0]=0;
                }
        }
        delete[] bufer;
        return info;
}

infofromserv findoutmyinfo(int sd,infofromserv stat)
{
        char *bufer;
        bufer= new char[length];
        bufer[0]=0;
        dprintf(sd,"mystatus\r\n");
        stat.iter=0;
        while(stat.iter<str5read)
        {
                if(!(testent(bufer)))
                        bufer=rdinbuf(bufer,sd);
                bufer=anrd(bufer,stat,ANALYS_YOURSELF);
        }
        delete[] bufer;
        return stat;
}

infofromserv findoutenemyinfo(int sd,infofromserv stat,int i)
{
        char *bufer;
        bufer= new char[length];
        bufer[0]=0;
        dprintf(sd,"playernumber %d\r\n",i);
        stat.iter=0;
	stat.enemynum=i;
        while(stat.iter<str5read)
        {
                if(!(testent(bufer)))
                        bufer=rdinbuf(bufer,sd);
                bufer=anrd(bufer,stat,ANALYS_ENEMYSELF);
        }
        delete[] bufer;
        return stat;
}

int connecttoserv(char *caddr,char *cport)
{
	int sd,port;
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	port=convnum(cport);
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=htonl(convnum(caddr));
	sd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==connect(sd,(struct sockaddr*)&addr,sizeof(addr))){
		perror("connect error");
		return 0;
	}
	return sd;
}

infofromserv prepareinfo(int sd)
{
	infofromserv info;
        info=findoutmarinfo(sd,info);
	if(!info.monthnum){
		info.enemyraw=new int[info.enemyarmy];
		info.enemyprod=new int[info.enemyarmy];
		info.enemypenny=new int[info.enemyarmy];
		info.enemyfact=new int[info.enemyarmy];
		info.auctraw=new int[info.enemyarmy];
		info.auctprod=new int[info.enemyarmy];
		info.auctrawpr=new int[info.enemyarmy];
		info.auctprodpr=new int[info.enemyarmy];
	}
	info=findoutmyinfo(sd,info);
	for(int i=0;i<info.enemyarmy;i++)
		info=findoutenemyinfo(sd,info,i);
	return info;
}

infofromserv rzero(infofromserv info,data4serv &data)
{
	for(int i=0;i<info.enemyarmy;i++){
		info.auctraw[i]=0;
		info.auctprod[i]=0;
		info.auctrawpr[i]=0;
		info.auctprodpr[i]=0;
	}
	data.sellpric=0;
	data.sellnum=0;
	data.buypric=0;
	data.buynum=0;
	data.numfact=0;
	data.makenum=0;
	data.endturn=0;
	return info;
}

void initdata(data4serv &data)
{
	data.sellpric=0;
	data.sellnum=0;
	data.buypric=0;
	data.buynum=0;
	data.numfact=0;
	data.makenum=0;
	data.endturn=0;
}

void syntanalys(token *tmas,int sd)
{
	infofromserv info;
	RPNItem *stack,*rpn,*rpnexe;
	data4serv data;
	syntacticobj synt(tmas,&info,&data);
	info.result=0;
	synt.analys();
	synt.second_path();
	rpn=synt.getrpn();
	initinfo(info);
	initdata(data);
	while(!info.result)
	{
		info=prepareinfo(sd);
		synt.takeinfo(&info);
		synt.takedata(&data);
		stack=NULL;
		rpnexe=rpn;
		while(rpnexe)
		{
			(rpnexe->elem)->Evaluate(&stack,&rpnexe);
		}
		data=synt.getdata();
		senddatatoserv(data,sd);
		info=rzero(info,data);
		info=analysauction(sd,info);
	}
	if(info.result==1)
		printf("i'm lose\n");
	if(info.result==2)
		printf("Eeeeeeeeeee! I'm winner!\n");
}

token *addnewtoken(token *tmas, int &len, machine &eng)
{
	if(!(len%10))
		tmas=moretok(tmas,len);
	tmas[len].joinb(eng.getbuf());
	tmas[len].givenum(eng.getstr());
	tmas[len].givetype(findol(eng.getbuf()));
	eng.cutbuf();
	if(tmas[len].gettype()==string)
		tmas[len].makenorm();
	len++;
	return tmas;
}

int main(int argc, char **argv)
{
	FILE *f;
	int g,t1,sd;
	machine eng;
	token *tmas;
	bool b;
	int len=0;
	const char *c="line number";
	const char *c1="Error in token:";
	sd=connecttoserv(argv[2],argv[3]);
	testargc(argc);
	f=fopen(argv[1],"r");
	testfilvar(f,argv[1]);
	g=fgetc(f);
	while(g!=EOF)
	{
		b=eng.feed(g);
		if(b)
			tmas=addnewtoken(tmas,len,eng);
		testquot(eng);
		g=fgetc(f);
	}
	outmas(tmas,len);
	try{
		syntanalys(tmas,sd);
	}
	catch(exeption &ex){
		t1=ex.getnum();
		printf("%s %d %s %s: %s\n",c,t1,c1,ex.gettok(),ex.geterr()); 
		return 1;
	}
	catch(const char *ex){
		printf("%s\n",ex); 
		return 1;
	}
	return 0;
}
