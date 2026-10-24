from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
import json
import os
import subprocess

app = Flask(__name__)
CORS(app)
PROJECT_FOLDER = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BRIDGE_SOURCE = os.path.join(PROJECT_FOLDER, "web_bridge.c")
BRIDGE_FILE = os.path.join(PROJECT_FOLDER, "web_bridge")


def ensure_bridge():
    if not os.path.exists(BRIDGE_FILE) or os.path.getmtime(BRIDGE_SOURCE) > os.path.getmtime(BRIDGE_FILE):
        result = subprocess.run(["cc", BRIDGE_SOURCE, "-o", BRIDGE_FILE], cwd=PROJECT_FOLDER, capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError(result.stderr or "Could not compile web_bridge.c")


def run_bridge(*args):
    ensure_bridge()
    result = subprocess.run([BRIDGE_FILE, *map(str, args)], cwd=PROJECT_FOLDER, capture_output=True, text=True, timeout=8)
    try:
        data = json.loads(result.stdout)
    except json.JSONDecodeError:
        raise RuntimeError(result.stderr or result.stdout or "Bridge returned invalid response")
    if result.returncode:
        return data, 400
    return data, 200


def error(message, code=400):
    return jsonify({"error": message}), code


@app.route("/")
def home():
    return send_from_directory(
        os.path.join(PROJECT_FOLDER, "frontend"),
        "index.html"
    )
@app.route("/api/status")
def status():
    return jsonify({"status": "success", "project": "SmartStock"})


@app.route("/api/products", methods=["GET"])
def get_products():
    try:
        data, code = run_bridge()
        return jsonify(data), code
    except Exception as exc:
        return error(str(exc), 500)


@app.route("/api/products", methods=["POST"])
def add_product():
    data = request.get_json(silent=True) or {}
    try:
        product_id = int(data.get("id", 0))
        quantity = int(data.get("quantity", -1))
        price = float(data.get("price", -1))
        name = str(data.get("name", "")).strip()
        category = str(data.get("category", "")).strip()
        if product_id <= 0 or quantity < 0 or price < 0 or not name or not category:
            return error("Enter a valid ID, name, category, price and quantity.")
        if len(name) >= 50 or len(category) >= 30:
            return error("Name must be under 50 characters and category under 30.")
        result, code = run_bridge("add", product_id, name, price, quantity, category)
        return jsonify(result), code
    except (TypeError, ValueError):
        return error("ID, price and quantity must be valid numbers.")
    except Exception as exc:
        return error(str(exc), 500)


@app.route("/api/products/<int:product_id>", methods=["PUT", "DELETE"])
def modify_product(product_id):
    try:
        if request.method == "DELETE":
            result, code = run_bridge("delete", product_id)
        else:
            data = request.get_json(silent=True) or {}
            name = str(data.get("name", "")).strip()
            category = str(data.get("category", "")).strip()
            price = float(data.get("price", -1))
            quantity = int(data.get("quantity", -1))
            if not name or not category or len(name) >= 50 or len(category) >= 30 or price < 0 or quantity < 0:
                return error("Enter valid product details.")
            result, code = run_bridge("update", product_id, name, price, quantity, category)
        return jsonify(result), code
    except (TypeError, ValueError):
        return error("Price and quantity must be valid numbers.")
    except Exception as exc:
        return error(str(exc), 500)


@app.route("/api/sell", methods=["POST"])
def sell_product():
    data = request.get_json(silent=True) or {}
    try:
        product_id = int(data.get("product_id", 0))
        quantity = int(data.get("quantity", 0))
        if product_id <= 0 or quantity <= 0:
            return error("Valid product ID and quantity required.")
        result, code = run_bridge("sell", product_id, quantity)
        return jsonify(result), code
    except (TypeError, ValueError):
        return error("Product ID and quantity must be numbers.")
    except Exception as exc:
        return error(str(exc), 500)


@app.route("/api/sales", methods=["GET"])
def sales_history():
    try:
        data, code = run_bridge("sales")
        return jsonify(data), code
    except Exception as exc:
        return error(str(exc), 500)


if __name__ == "__main__":
    app.run(debug=True, port=5000)
