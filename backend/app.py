
from flask import Flask, jsonify, request
from flask_cors import CORS
import subprocess
import os

app = Flask(__name__)
CORS(app)

# SmartStock project folder
PROJECT_FOLDER = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BRIDGE_FILE = os.path.join(PROJECT_FOLDER, "web_bridge")


@app.route("/")
def home():
    return jsonify({
        "message": "SmartStock Backend is running!"
    })


@app.route("/api/status")
def status():
    return jsonify({
        "status": "success",
        "project": "SmartStock"
    })


@app.route("/api/products")
def get_products():
    try:
        result = subprocess.run(
            [BRIDGE_FILE],
            cwd=PROJECT_FOLDER,
            capture_output=True,
            text=True,
            timeout=5,
            check=True
        )

        import json
        return jsonify(json.loads(result.stdout))

    except Exception as error:
        return jsonify({
            "error": "Could not load products",
            "details": str(error)
        }), 500


@app.route("/api/sell", methods=["POST"])
def sell_product():
    try:
        data = request.get_json()
        product_id = int(data.get("product_id", 0))
        quantity = int(data.get("quantity", 0))

        if product_id <= 0 or quantity <= 0:
            return jsonify({"error": "Valid product ID and quantity required"}), 400

        result = subprocess.run(
            [BRIDGE_FILE, "sell", str(product_id), str(quantity)],
            cwd=PROJECT_FOLDER,
            capture_output=True,
            text=True,
            timeout=5
        )

        import json
        response = json.loads(result.stdout)

        if result.returncode != 0:
            return jsonify({
                "error": response.get("error", "Sale failed")
            }), 400

        return jsonify({
            "message": response.get("message", "Sale completed successfully")
        })

    except Exception as error:
        return jsonify({"error": str(error)}), 500


if __name__ == "__main__":
    app.run(debug=True, port=5000)
