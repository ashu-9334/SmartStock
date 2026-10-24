#include <stdio.h>
#include <string.h>

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



/* Save products to file */
void saveProducts(struct Product products[], int count) {
    FILE *file = fopen("products.dat", "wb");

    if (file == NULL) {
        printf("\nError saving products!\n");
        return;
    }

    fwrite(&count, sizeof(int), 1, file);
    fwrite(products, sizeof(struct Product), count, file);

    fclose(file);
}

/* Load products from file */
void loadProducts(struct Product products[], int *count) {
    FILE *file = fopen("products.dat", "rb");

    if (file == NULL) {
        *count = 0;
        return;
    }

    fread(count, sizeof(int), 1, file);

    if (*count < 0 || *count > 100) {
        *count = 0;
        fclose(file);
        return;
    }

    fread(products, sizeof(struct Product), *count, file);

    fclose(file);
}


/* Save sales record to file */
void saveSale(struct Sale sale) {
    FILE *file = fopen("sales.dat", "ab");

    if (file == NULL) {
        printf("\nError saving sales record!\n");
        return;
    }

    fwrite(&sale, sizeof(struct Sale), 1, file);

    fclose(file);
}



int main() {

    struct Product products[100];
    int count = 0;
    int choice;
    int searchId;
    int found;
    loadProducts(products, &count);

    do {

        printf("\n============================================\n");
        printf("              SMARTSTOCK\n");
        printf("   Inventory & Sales Management System\n");
        printf("============================================\n");

        printf("\n1. Add Product");
        printf("\n2. View Products");
        printf("\n3. Search Product");
        printf("\n4. Update Product");
        printf("\n5. Delete Product");
        printf("\n6. Sell Product");
        printf("\n7. Generate Bill");
        printf("\n8. Sales Report");
        printf("\n9. Low Stock Alert");
        printf("\n0. Exit");

        printf("\n\nEnter your choice: ");
        scanf("%d", &choice);

        
        
        /* Add Product */
        if (choice == 1) {

            printf("\n----------- ADD PRODUCT -----------\n");

            if (count >= 100) {
                printf("Inventory is full! Cannot add more products.\n");
            }
            else {
                int duplicate = 0;
                int newId;

                printf("Enter Product ID: ");
                scanf("%d", &newId);

                for (int i = 0; i < count; i++) {
                    if (products[i].id == newId) {
                        duplicate = 1;
                        break;
                    }
                }

                if (duplicate == 1) {
                    printf("\nError: Product ID already exists!\n");
                }
                else {
                    products[count].id = newId;

                    printf("Enter Product Name: ");
                    scanf(" %[^\n]", products[count].name);

                    printf("Enter Category: ");
                    scanf(" %[^\n]", products[count].category);

                    printf("Enter Price: ");
                    scanf("%f", &products[count].price);

                    if (products[count].price <= 0) {
                        printf("\nInvalid price! Price must be greater than zero.\n");
                    }
                    else {
                        printf("Enter Quantity: ");
                        scanf("%d", &products[count].quantity);

                        if (products[count].quantity < 0) {
                            printf("\nInvalid quantity! Cannot be negative.\n");
                        }
                        else {
                            count++;

                            saveProducts(products, count);

                            printf("\nProduct Added Successfully!\n");
                        }
                    }
                }
            }
        }



        /* View Products */
        else if (choice == 2) {

            printf("\n----------- PRODUCT LIST -----------\n");

            if (count == 0) {
                printf("No products available.\n");
            }
            else {

                for (int i = 0; i < count; i++) {

                    printf("\nProduct %d\n", i + 1);
                    printf("ID       : %d\n", products[i].id);
                    printf("Name     : %s\n", products[i].name);
                    printf("Category : %s\n", products[i].category);
                    printf("Price    : %.2f\n", products[i].price);
                    printf("Quantity : %d\n", products[i].quantity);
                }
            }
        }

        /* Search Product */
        else if (choice == 3) {

            char searchName[50];

            printf("\n----------- SEARCH PRODUCT -----------\n");

            printf("Enter Product Name: ");
            scanf(" %[^\n]", searchName);

            found = 0;

            for (int i = 0; i < count; i++) {

                if (strcmp(products[i].name, searchName) == 0) {

                    printf("\nProduct Found!\n");
                    printf("----------------------------\n");
                    printf("ID       : %d\n", products[i].id);
                    printf("Name     : %s\n", products[i].name);
                    printf("Category : %s\n", products[i].category);
                    printf("Price    : %.2f\n", products[i].price);
                    printf("Quantity : %d\n", products[i].quantity);

                    found = 1;
                    break;
                }
            }

            if (found == 0) {
                printf("\nProduct not found!\n");
            }
        }

        /* Update Product */
        else if (choice == 4) {

            printf("\n----------- UPDATE PRODUCT -----------\n");

            printf("Enter Product ID: ");
            scanf("%d", &searchId);

            found = 0;

            for (int i = 0; i < count; i++) {

                if (products[i].id == searchId) {

                    printf("\nProduct Found!\n");

                    printf("Current Price: %.2f\n", products[i].price);
                    printf("Enter New Price: ");
                    scanf("%f", &products[i].price);

                    printf("Current Quantity: %d\n", products[i].quantity);
                    printf("Enter New Quantity: ");
                    scanf("%d", &products[i].quantity);

                    saveProducts(products, count);
                    printf("\nProduct Updated Successfully!\n");
                    

                    found = 1;
                    break;
                }
            }

            if (found == 0) {
                printf("\nProduct not found!\n");
            }
        }

        /* Delete Product */
        else if (choice == 5) {

            printf("\n----------- DELETE PRODUCT -----------\n");

            printf("Enter Product ID: ");
            scanf("%d", &searchId);

            found = 0;

            for (int i = 0; i < count; i++) {

                if (products[i].id == searchId) {

                    for (int j = i; j < count - 1; j++) {
                        products[j] = products[j + 1];
                    }

                    count--;
                    saveProducts(products, count);

                    printf("\nProduct Deleted Successfully!\n");

                    found = 1;
                    break;
                }
            }

            if (found == 0) {
                printf("\nProduct not found!\n");
            }
        }

        /* Sell Product */
        else if (choice == 6) {

            int sellId;
            int sellQuantity;
            float totalAmount;

            printf("\n----------- SELL PRODUCT -----------\n");

            printf("Enter Product ID: ");
            scanf("%d", &sellId);

            found = 0;

            for (int i = 0; i < count; i++) {

                if (products[i].id == sellId) {

                    found = 1;

                    printf("Product Name: %s\n", products[i].name);
                    printf("Price: %.2f\n", products[i].price);
                    printf("Available Quantity: %d\n", products[i].quantity);

                    printf("Enter Quantity to Sell: ");
                    scanf("%d", &sellQuantity);

                    if (sellQuantity <= 0) {
                        printf("\nInvalid quantity!\n");
                    }
                    else if (sellQuantity > products[i].quantity) {
                        printf("\nInsufficient stock!\n");
                    }
                    else {

                        
                        totalAmount = products[i].price * sellQuantity;

                        /* Create sales record */
                        struct Sale sale;

                        sale.productId = products[i].id;
                        strcpy(sale.productName, products[i].name);
                        sale.quantitySold = sellQuantity;
                        sale.price = products[i].price;
                        sale.totalAmount = totalAmount;

                        /* Save sales record */
                        saveSale(sale);

                        /* Update and save remaining stock */
                        products[i].quantity =
                            products[i].quantity - sellQuantity;

                        saveProducts(products, count);


                        printf("\nSale Successful!\n");
                        printf("----------------------------\n");
                        printf("Product  : %s\n", products[i].name);
                        printf("Quantity : %d\n", sellQuantity);
                        printf("Total    : %.2f\n", totalAmount);
                        printf("Remaining Stock : %d\n", products[i].quantity);
                    }

                    break;
                }
            }

            if (found == 0) {
                printf("\nProduct not found!\n");
            }
        }

        /* Generate Bill */
        else if (choice == 7) {

            int billId;
            int billQuantity;
            float totalAmount;

            printf("\n----------- GENERATE BILL -----------\n");

            printf("Enter Product ID: ");
            scanf("%d", &billId);

            found = 0;

            for (int i = 0; i < count; i++) {

                if (products[i].id == billId) {

                    found = 1;

                    printf("Enter Quantity: ");
                    scanf("%d", &billQuantity);

                    if (billQuantity <= 0) {
                        printf("\nInvalid quantity!\n");
                    }
                    else if (billQuantity > products[i].quantity) {
                        printf("\nInsufficient stock!\n");
                    }
                    else {

                        
                        totalAmount = products[i].price * billQuantity;

                        /* Create sales record */
                        struct Sale sale;

                        sale.productId = products[i].id;
                        strcpy(sale.productName, products[i].name);
                        sale.quantitySold = billQuantity;
                        sale.price = products[i].price;
                        sale.totalAmount = totalAmount;

                        /* Save sales record */
                        saveSale(sale);

                        /* Update and save remaining stock */
                        products[i].quantity -= billQuantity;
                        saveProducts(products, count);


                        printf("\nStock updated successfully!\n");
                        printf("Remaining Stock: %d\n", products[i].quantity);

                        printf("\n============================================\n");
                        printf("              SMARTSTOCK BILL\n");
                        printf("============================================\n");
                        printf("Product ID     : %d\n", products[i].id);
                        printf("Product Name   : %s\n", products[i].name);
                        printf("Category       : %s\n", products[i].category);
                        printf("Price          : %.2f\n", products[i].price);
                        printf("Quantity       : %d\n", billQuantity);
                        printf("--------------------------------------------\n");
                        printf("Total Amount   : %.2f\n", totalAmount);
                        printf("============================================\n");
                        printf("          Thank You! Visit Again!\n");
                        printf("============================================\n");
                    }

                    break;
                }
            }

            if (found == 0) {
                printf("\nProduct not found!\n");
            }
        }

        
        /* Sales Report */
        else if (choice == 8) {

            FILE *file;
            struct Sale sale;
            int totalSales = 0;
            int totalQuantitySold = 0;
            float totalRevenue = 0;

            printf("\n============================================\n");
            printf("                 SALES REPORT\n");
            printf("============================================\n");

            file = fopen("sales.dat", "rb");

            if (file == NULL) {
                printf("No sales records found.\n");
            }
            else {

                printf("\n%-8s %-18s %-8s %-10s %-12s\n",
                    "ID", "Product", "Qty Sold", "Price", "Total");

                printf("------------------------------------------------------------\n");

                while (fread(&sale, sizeof(struct Sale), 1, file) == 1) {

                    printf("%-8d %-18s %-8d %-10.2f %-12.2f\n",
                        sale.productId,
                        sale.productName,
                        sale.quantitySold,
                        sale.price,
                        sale.totalAmount);

                    totalSales++;
                    totalQuantitySold += sale.quantitySold;
                    totalRevenue += sale.totalAmount;
                }

                fclose(file);

                printf("------------------------------------------------------------\n");
                printf("Number of Sales   : %d\n", totalSales);
                printf("Total Items Sold  : %d\n", totalQuantitySold);
                printf("Total Revenue     : %.2f\n", totalRevenue);
                printf("============================================\n");
            }
        }


        
        /* Low Stock Alert */
        else if (choice == 9) {

            int lowStockFound = 0;

            printf("\n========== LOW STOCK ALERT ==========\n");

            for (int i = 0; i < count; i++) {

                if (products[i].quantity <= 5) {

                    printf("\nProduct ID : %d\n", products[i].id);
                    printf("Name       : %s\n", products[i].name);
                    printf("Quantity   : %d\n", products[i].quantity);
                    printf("-------------------------------------\n");

                    lowStockFound = 1;
                }
            }

            if (lowStockFound == 0) {
                printf("All products have sufficient stock!\n");
            }
        }



        /* Exit */
        else if (choice == 0) {

            printf("\nThank you for using SmartStock!\n");
        }

        else {

            printf("\nInvalid choice! Please try again.\n");
        }

    } while (choice != 0);

    return 0;
}