

async function loadProducts() {
    try {
        const response = await fetch("http://127.0.0.1:5000/api/products");

        if (!response.ok) {
            throw new Error("Could not load products");
        }

        const data = await response.json();
        const products = data.products;

        const tableBody = document.getElementById("productTableBody");
        tableBody.innerHTML = "";

        let totalStock = 0;

        products.forEach(function (product) {
            totalStock += product.quantity;

            const row = document.createElement("tr");

            [
                product.id,
                product.name,
                product.category,
                Number(product.price).toFixed(2),
                product.quantity
            ].forEach(function (value) {
                const cell = document.createElement("td");
                cell.textContent = value;
                row.appendChild(cell);
            });

            tableBody.appendChild(row);
        });

        document.getElementById("totalProducts").textContent = products.length;
        document.getElementById("totalStock").textContent = totalStock;

        document.getElementById("totalRevenue").textContent =
            "₹" + Number(data.sales.totalRevenue).toLocaleString("en-IN");

        document.getElementById("totalSales").textContent =
            data.sales.totalSales;

        document.getElementById("totalItemsSold").textContent =
            data.sales.totalItemsSold;


    } catch (error) {
        console.error("Error loading products:", error);

        document.getElementById("productTableBody").innerHTML =
            '<tr><td colspan="5">Could not load products. Check the backend.</td></tr>';
    }
}

loadProducts();

document.getElementById("viewProductsBtn").addEventListener("click", function () {
    document.getElementById("inventory").scrollIntoView({
        behavior: "smooth"
    });
});


const sellProductButton = document.getElementById("sellProductBtn");
const saleFormSection = document.getElementById("saleFormSection");
const cancelSaleButton = document.getElementById("cancelSaleBtn");
const saleForm = document.getElementById("saleForm");

sellProductButton.addEventListener("click", function () {
    saleFormSection.hidden = false;

    saleFormSection.scrollIntoView({
        behavior: "smooth",
        block: "start"
    });
});

cancelSaleButton.addEventListener("click", function () {
    saleForm.reset();
    saleFormSection.hidden = true;
    document.getElementById("saleMessage").textContent = "";
});

document.getElementById("saleForm").addEventListener("submit", async function (event) {
    event.preventDefault();

    const productId = document.getElementById("saleProductId").value;
    const quantity = document.getElementById("saleQuantity").value;
    const message = document.getElementById("saleMessage");

    try {
        const response = await fetch("http://127.0.0.1:5000/api/sell", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({
                product_id: Number(productId),
                quantity: Number(quantity)
            })
        });

        const result = await response.json();
        message.textContent = result.message || result.error || "Sale failed.";

        if (response.ok) {
            document.getElementById("saleForm").reset();
            await loadProducts();
        }
    } catch (error) {
        message.textContent = "Backend start karo aur dobara try karo.";
    }
});