const API = "/api";
let productsCache = [];
const $ = (id) => document.getElementById(id);
const money = (value) => "₹" + Number(value || 0).toLocaleString("en-IN", {minimumFractionDigits: 2, maximumFractionDigits: 2});
const esc = (value) => String(value ?? "").replace(/[&<>"']/g, ch => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[ch]));

async function api(path, options = {}) {
  const response = await fetch(API + path, {headers: {"Content-Type":"application/json", ...(options.headers || {})}, ...options});
  const data = await response.json().catch(() => ({}));
  if (!response.ok) throw new Error(data.error || data.message || "Request failed");
  return data;
}
function showMessage(id, text, success = false) { const el=$(id); el.textContent=text; el.className="message " + (success ? "success" : "error"); }
function showPage(page) {
  document.querySelectorAll(".page-section").forEach(section => section.classList.toggle("hidden", section.id !== `page-${page}`));
  document.querySelectorAll(".nav-link").forEach(link => link.classList.toggle("active", link.dataset.page === page));
  const titles={dashboard:"Dashboard Overview",products:"Product Management",sales:"Sales Management",inventory:"Inventory & Stock"};
  $("pageTitle").textContent=titles[page] || "Dashboard Overview"; $("breadcrumbPage").textContent=titles[page] || "Dashboard";
  if(page === "sales") loadSales();
  if(page === "products" || page === "inventory") loadProducts();
  window.scrollTo({top:0,behavior:"smooth"});
}
document.querySelectorAll(".nav-link").forEach(link => link.addEventListener("click",()=>showPage(link.dataset.page)));
document.querySelectorAll("[data-go]").forEach(button=>button.addEventListener("click",()=>showPage(button.dataset.go)));
$("quickAddBtn").addEventListener("click",()=>{showPage("products");resetProductForm();$("productName").focus();});
$("quickSellBtn").addEventListener("click",()=>showPage("sales"));
$("quickSalesBtn").addEventListener("click",()=>showPage("sales"));

async function loadProducts() {
  try {
    const data=await api("/products"); productsCache=data.products || []; const sales=data.sales || {};
    $("totalProducts").textContent=productsCache.length;
    $("totalStock").textContent=productsCache.reduce((sum,p)=>sum+Number(p.quantity||0),0);
    $("totalRevenue").textContent=money(sales.totalRevenue);
    $("totalSales").textContent=sales.totalSales || 0; $("totalItemsSold").textContent=sales.totalItemsSold || 0;
    renderProducts(); renderDashboardProducts(); renderInventory(); populateSaleProducts();
  } catch(error) {
    ["dashboardProducts","productsTable","inventoryTable"].forEach(id=>{if($(id))$(id).innerHTML=`<tr><td colspan="6">${esc(error.message)}. Start the backend and refresh.</td></tr>`;});
  }
}
function renderDashboardProducts(){const body=$("dashboardProducts");body.innerHTML=productsCache.slice(0,6).map(p=>`<tr><td>${p.id}</td><td>${esc(p.name)}</td><td>${esc(p.category)}</td><td>${money(p.price)}</td><td>${p.quantity}</td></tr>`).join("")||'<tr><td colspan="5">No products yet.</td></tr>';}
function renderProducts(){const body=$("productsTable");if(!body)return;const q=($("productSearch").value||"").toLowerCase();const list=productsCache.filter(p=>`${p.id} ${p.name} ${p.category}`.toLowerCase().includes(q));body.innerHTML=list.map(p=>`<tr><td>${p.id}</td><td>${esc(p.name)}</td><td>${esc(p.category)}</td><td>${money(p.price)}</td><td>${p.quantity}</td><td><div class="row-actions"><button type="button" data-edit="${p.id}">Edit</button><button type="button" class="delete-btn" data-delete="${p.id}">Delete</button></div></td></tr>`).join("")||'<tr><td colspan="6">No matching products.</td></tr>';body.querySelectorAll("[data-edit]").forEach(b=>b.addEventListener("click",()=>editProduct(Number(b.dataset.edit))));body.querySelectorAll("[data-delete]").forEach(b=>b.addEventListener("click",()=>deleteProduct(Number(b.dataset.delete))));}
function renderInventory(){const body=$("inventoryTable");if(!body)return;const q=($("inventorySearch").value||"").toLowerCase();const list=productsCache.filter(p=>`${p.id} ${p.name} ${p.category}`.toLowerCase().includes(q));body.innerHTML=list.map(p=>`<tr><td>${p.id}</td><td>${esc(p.name)}</td><td>${esc(p.category)}</td><td>${money(p.price)}</td><td>${p.quantity}</td><td class="${p.quantity<=5?'stock-low':'stock-good'}">${p.quantity<=5?'Low stock':'In stock'}</td></tr>`).join("")||'<tr><td colspan="6">No matching products.</td></tr>';}
function populateSaleProducts(){const select=$("saleProductId");if(!select)return;const current=select.value;select.innerHTML='<option value="">Choose a product</option>'+productsCache.filter(p=>p.quantity>0).map(p=>`<option value="${p.id}">${esc(p.name)} (ID ${p.id}) — ${p.quantity} available</option>`).join("");if(productsCache.some(p=>String(p.id)===current&&p.quantity>0))select.value=current;}
$("productSearch").addEventListener("input",renderProducts);$("inventorySearch").addEventListener("input",renderInventory);
function resetProductForm(){ $("productForm").reset();$("editingProductId").value="";$("productId").disabled=false;$("productFormTitle").textContent="Add Product";$("saveProductBtn").textContent="Add Product";$("cancelEditBtn").classList.add("hidden");$("productMessage").textContent=""; }
function editProduct(id){const p=productsCache.find(item=>item.id===id);if(!p)return;showPage("products");$("editingProductId").value=p.id;$("productId").value=p.id;$("productId").disabled=true;$("productName").value=p.name;$("productCategory").value=p.category;$("productPrice").value=p.price;$("productQuantity").value=p.quantity;$("productFormTitle").textContent="Edit Product";$("saveProductBtn").textContent="Save Changes";$("cancelEditBtn").classList.remove("hidden");window.scrollTo({top:0,behavior:"smooth"});}
$("cancelEditBtn").addEventListener("click",resetProductForm);
$("productForm").addEventListener("submit",async event=>{event.preventDefault();const editing=$("editingProductId").value;const payload={id:Number($("productId").value),name:$("productName").value.trim(),category:$("productCategory").value.trim(),price:Number($("productPrice").value),quantity:Number($("productQuantity").value)};try{let result;if(editing)result=await api(`/products/${editing}`,{method:"PUT",body:JSON.stringify(payload)});else result=await api("/products",{method:"POST",body:JSON.stringify(payload)});showMessage("productMessage",result.message||"Saved successfully",true);resetProductForm();await loadProducts();}catch(error){showMessage("productMessage",error.message);}});
async function deleteProduct(id){const p=productsCache.find(item=>item.id===id);if(!p)return;if(!confirm(`Delete product “${p.name}” (ID ${id})? This removes it from current inventory.`))return;try{const result=await api(`/products/${id}`,{method:"DELETE"});await loadProducts();showMessage("productMessage",result.message||"Product deleted",true);}catch(error){alert(error.message);}}
$("saleForm").addEventListener("submit",async event=>{event.preventDefault();const product_id=Number($("saleProductId").value);const quantity=Number($("saleQuantity").value);try{const result=await api("/sell",{method:"POST",body:JSON.stringify({product_id,quantity})});showMessage("saleMessage",`${result.message||"Sale completed"}${result.totalAmount!==undefined?" — Total: "+money(result.totalAmount):""}`,true);$("saleForm").reset();await loadProducts();await loadSales();}catch(error){showMessage("saleMessage",error.message);}});
async function loadSales(){try{const data=await api("/sales");const body=$("salesTable");body.innerHTML=(data.sales||[]).slice().reverse().map(s=>`<tr><td>${s.productId}</td><td>${esc(s.productName)}</td><td>${s.quantity}</td><td>${money(s.price)}</td><td>${money(s.total)}</td></tr>`).join("")||'<tr><td colspan="5">No sales recorded yet.</td></tr>';}catch(error){$("salesTable").innerHTML=`<tr><td colspan="5">${esc(error.message)}</td></tr>`;}}
$("refreshSalesBtn").addEventListener("click",()=>{loadSales();loadProducts();});
loadProducts();
