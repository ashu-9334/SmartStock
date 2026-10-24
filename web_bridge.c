
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

struct Product {
    int id;
    char name[50];
    char category[30];
    float price;
    int quantity;
};

struct Sale {
    int productId;
    char productName[50];
    int quantitySold;
    float price;
    float totalAmount;
};

int saveProducts(struct Product products[], int count) {
    FILE *file = fopen("products.dat", "wb");
    if (!file) return 0;

    int ok = fwrite(&count, sizeof(int), 1, file) == 1 &&
             fwrite(products, sizeof(struct Product), count, file)
                 == (size_t)count;

    if (fclose(file) != 0) ok = 0;
    return ok;
}

int readNumber(const char *text, int *number) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);

    if (errno || text == end || *end != '\0' ||
        value < 1 || value > INT_MAX) return 0;

    *number = (int)value;
    return 1;
}

int processSale(int productId, int quantity) {
    struct Product products[100];
    int count;
    FILE *file = fopen("products.dat", "rb");

    if (!file) {
        printf("{\"error\":\"Cannot open products.dat\"}\n");
        return 1;
    }

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > 100 ||
        fread(products, sizeof(struct Product), count, file)
            != (size_t)count) {
        fclose(file);
        printf("{\"error\":\"Invalid product data\"}\n");
        return 1;
    }
    fclose(file);

    int index = -1;
    for (int i = 0; i < count; i++) {
        if (products[i].id == productId) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        printf("{\"error\":\"Product not found\"}\n");
        return 1;
    }

    if (quantity > products[index].quantity) {
        printf("{\"error\":\"Insufficient stock\"}\n");
        return 1;
    }

    struct Sale sale = {0};
    sale.productId = products[index].id;
    snprintf(sale.productName, sizeof(sale.productName), "%s",
             products[index].name);
    sale.quantitySold = quantity;
    sale.price = products[index].price;
    sale.totalAmount = sale.price * quantity;

    FILE *salesFile = fopen("sales.dat", "ab");
    if (!salesFile) {
        printf("{\"error\":\"Cannot open sales.dat\"}\n");
        return 1;
    }

    if (fwrite(&sale, sizeof(struct Sale), 1, salesFile) != 1 ||
        fflush(salesFile) != 0) {
        fclose(salesFile);
        printf("{\"error\":\"Could not save sale\"}\n");
        return 1;
    }

    if (fclose(salesFile) != 0) {
        printf("{\"error\":\"Could not close sales.dat\"}\n");
        return 1;
    }

    products[index].quantity -= quantity;

    if (!saveProducts(products, count)) {
        printf("{\"error\":\"Sale saved but stock update failed\"}\n");
        return 1;
    }

    printf("{\"message\":\"Sale completed\",\"productId\":%d,"
           "\"quantitySold\":%d,\"totalAmount\":%.2f,"
           "\"remainingStock\":%d}\n",
           productId, quantity, sale.totalAmount,
           products[index].quantity);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 4 && strcmp(argv[1], "sell") == 0) {
        int productId, quantity;

        if (!readNumber(argv[2], &productId) ||
            !readNumber(argv[3], &quantity)) {
            printf("{\"error\":\"Invalid product ID or quantity\"}\n");
            return 1;
        }

        return processSale(productId, quantity);
    }

    if (argc != 1) {
        printf("{\"error\":\"Invalid arguments\"}\n");
        return 1;
    }

    struct Product products[100];
    struct Sale sale;
    int count = 0, totalSales = 0, totalItemsSold = 0;
    double totalRevenue = 0.0;

    FILE *file = fopen("products.dat", "rb");
    if (!file) {
        printf("{\"error\":\"Could not open products.dat\"}\n");
        return 1;
    }

    if (fread(&count, sizeof(int), 1, file) != 1 ||
        count < 0 || count > 100 ||
        fread(products, sizeof(struct Product), count, file)
            != (size_t)count) {
        fclose(file);
        printf("{\"error\":\"Invalid product data\"}\n");
        return 1;
    }
    fclose(file);

    printf("{\"products\":[");
    for (int i = 0; i < count; i++) {
        printf("%s{\"id\":%d,\"name\":\"%.49s\","
               "\"category\":\"%.29s\",\"price\":%.2f,"
               "\"quantity\":%d}",
               i ? "," : "", products[i].id, products[i].name,
               products[i].category, products[i].price,
               products[i].quantity);
    }

    printf("],");
    file = fopen("sales.dat", "rb");

    if (file) {
        while (fread(&sale, sizeof(struct Sale), 1, file) == 1) {
            totalSales++;
            totalItemsSold += sale.quantitySold;
            totalRevenue += sale.totalAmount;
        }
        fclose(file);
    }

    printf("\"sales\":{\"totalSales\":%d,\"totalItemsSold\":%d,"
           "\"totalRevenue\":%.2f}}\n",
           totalSales, totalItemsSold, totalRevenue);
    return 0;
}
