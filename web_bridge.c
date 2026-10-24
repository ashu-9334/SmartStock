#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_PRODUCTS 100
struct Product { int id; char name[50]; char category[30]; float price; int quantity; };
struct Sale { int productId; char productName[50]; int quantitySold; float price; float totalAmount; };

static void jsonString(const char *s) {
    putchar('"');
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { putchar('\\'); putchar(c); }
        else if (c == '\n') printf("\\n");
        else if (c == '\r') printf("\\r");
        else if (c == '\t') printf("\\t");
        else if (c >= 32) putchar(c);
    }
    putchar('"');
}
static int number(const char *s, int allowZero, int *out) {
    char *end; errno = 0; long n = strtol(s, &end, 10);
    if (errno || s == end || *end || n < (allowZero ? 0 : 1) || n > INT_MAX) return 0;
    *out = (int)n; return 1;
}
static int readProducts(struct Product p[], int *count) {
    FILE *f = fopen("products.dat", "rb"); if (!f) return 0;
    int ok = fread(count, sizeof(int), 1, f) == 1 && *count >= 0 && *count <= MAX_PRODUCTS && fread(p, sizeof(struct Product), *count, f) == (size_t)*count;
    fclose(f); return ok;
}
static int saveProducts(struct Product p[], int count) {
    FILE *f = fopen("products.dat", "wb"); if (!f) return 0;
    int ok = fwrite(&count, sizeof(int), 1, f) == 1 && fwrite(p, sizeof(struct Product), count, f) == (size_t)count;
    if (fclose(f) != 0) ok = 0; return ok;
}
static int textCopy(char *dst, size_t cap, const char *src) {
    if (!src || !*src || strlen(src) >= cap) return 0;
    snprintf(dst, cap, "%s", src); return 1;
}
static void listProducts(void) {
    struct Product p[MAX_PRODUCTS]; int n;
    if (!readProducts(p, &n)) { printf("{\"error\":\"Could not read products.dat\"}\n"); exit(1); }
    printf("{\"products\":[");
    for (int i=0;i<n;i++) { if(i) putchar(','); printf("{\"id\":%d,\"name\":",p[i].id); jsonString(p[i].name); printf(",\"category\":"); jsonString(p[i].category); printf(",\"price\":%.2f,\"quantity\":%d}",p[i].price,p[i].quantity); }
    int totalSales=0,totalItems=0; double revenue=0; struct Sale s; FILE *f=fopen("sales.dat","rb");
    if(f){ while(fread(&s,sizeof(s),1,f)==1){totalSales++;totalItems+=s.quantitySold;revenue+=s.totalAmount;} fclose(f); }
    printf("],\"sales\":{\"totalSales\":%d,\"totalItemsSold\":%d,\"totalRevenue\":%.2f}}\n",totalSales,totalItems,revenue);
}
static int sell(int id,int qty) {
    struct Product p[MAX_PRODUCTS]; int n;
    if(!readProducts(p,&n)){printf("{\"error\":\"Could not read products.dat\"}\n");return 1;}
    int i; for(i=0;i<n && p[i].id!=id;i++); if(i==n){printf("{\"error\":\"Product not found\"}\n");return 1;}
    if(qty>p[i].quantity){printf("{\"error\":\"Insufficient stock\"}\n");return 1;}
    struct Sale s={0}; s.productId=id; snprintf(s.productName,sizeof(s.productName),"%s",p[i].name); s.quantitySold=qty; s.price=p[i].price; s.totalAmount=s.price*qty;
    FILE *f=fopen("sales.dat","ab"); if(!f){printf("{\"error\":\"Cannot open sales.dat\"}\n");return 1;}
    int ok=fwrite(&s,sizeof(s),1,f)==1; if(fclose(f)!=0)ok=0; if(!ok){printf("{\"error\":\"Could not save sale\"}\n");return 1;}
    p[i].quantity-=qty; if(!saveProducts(p,n)){printf("{\"error\":\"Sale recorded but stock update failed; check data backup\"}\n");return 1;}
    printf("{\"message\":\"Sale completed successfully\",\"totalAmount\":%.2f,\"remainingStock\":%d}\n",s.totalAmount,p[i].quantity); return 0;
}
static int productMutation(int argc,char **argv) {
    struct Product p[MAX_PRODUCTS]; int n; if(!readProducts(p,&n)){printf("{\"error\":\"Could not read products.dat\"}\n");return 1;}
    if(strcmp(argv[1],"add")==0 && argc==7){
        int id,qty; char *end; errno=0; float price=strtof(argv[4],&end);
        if(!number(argv[2],0,&id)||!number(argv[5],1,&qty)||errno||*end||price<0||strlen(argv[3])==0||strlen(argv[3])>=50||strlen(argv[6])==0||strlen(argv[6])>=30){printf("{\"error\":\"Invalid product fields (name max 49, category max 29)\"}\n");return 1;}
        if(n>=MAX_PRODUCTS){printf("{\"error\":\"Product limit reached (100)\"}\n");return 1;} for(int i=0;i<n;i++)if(p[i].id==id){printf("{\"error\":\"Product ID already exists\"}\n");return 1;}
        struct Product x={0}; x.id=id; snprintf(x.name,sizeof(x.name),"%s",argv[3]); snprintf(x.category,sizeof(x.category),"%s",argv[6]); x.price=price; x.quantity=qty; p[n++]=x;
        if(!saveProducts(p,n)){printf("{\"error\":\"Could not save products\"}\n");return 1;} printf("{\"message\":\"Product added successfully\"}\n");return 0;
    }
    int id; if(argc<3||!number(argv[2],0,&id)){printf("{\"error\":\"Valid product ID required\"}\n");return 1;} int i; for(i=0;i<n&&p[i].id!=id;i++); if(i==n){printf("{\"error\":\"Product not found\"}\n");return 1;}
    if(strcmp(argv[1],"delete")==0 && argc==3){for(int j=i;j<n-1;j++)p[j]=p[j+1]; n--; if(!saveProducts(p,n)){printf("{\"error\":\"Could not save products\"}\n");return 1;} printf("{\"message\":\"Product deleted\"}\n");return 0;}
    if(strcmp(argv[1],"update")==0 && argc==7){int qty;char *end;errno=0;float price=strtof(argv[4],&end);if(!number(argv[5],1,&qty)||errno||*end||price<0||strlen(argv[3])==0||strlen(argv[3])>=50||strlen(argv[6])==0||strlen(argv[6])>=30){printf("{\"error\":\"Invalid product fields\"}\n");return 1;}snprintf(p[i].name,sizeof(p[i].name),"%s",argv[3]);p[i].price=price;p[i].quantity=qty;snprintf(p[i].category,sizeof(p[i].category),"%s",argv[6]);if(!saveProducts(p,n)){printf("{\"error\":\"Could not save products\"}\n");return 1;}printf("{\"message\":\"Product updated\"}\n");return 0;}
    printf("{\"error\":\"Invalid product command\"}\n");return 1;
}
static void salesHistory(void){struct Sale s;FILE*f=fopen("sales.dat","rb");printf("{\"sales\":[");int first=1;if(f){while(fread(&s,sizeof(s),1,f)==1){if(!first)putchar(',');first=0;printf("{\"productId\":%d,\"productName\":",s.productId);jsonString(s.productName);printf(",\"quantity\":%d,\"price\":%.2f,\"total\":%.2f}",s.quantitySold,s.price,s.totalAmount);}fclose(f);}printf("]}\n");}
int main(int argc,char **argv){if(argc==1){listProducts();return 0;}if(strcmp(argv[1],"sell")==0&&argc==4){int id,q;if(!number(argv[2],0,&id)||!number(argv[3],0,&q)||q==0){printf("{\"error\":\"Invalid product ID or quantity\"}\n");return 1;}return sell(id,q);}if(strcmp(argv[1],"sales")==0&&argc==2){salesHistory();return 0;}if(strcmp(argv[1],"add")==0||strcmp(argv[1],"update")==0||strcmp(argv[1],"delete")==0)return productMutation(argc,argv);printf("{\"error\":\"Invalid arguments\"}\n");return 1;}
