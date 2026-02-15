#include <iostream>
#include <string>
using namespace std;

// ================= Product Node (SLL) =================
class ProductNode {
public:
    int id;
    string name;
    double price;
    int quantity;
    ProductNode* next;

    ProductNode(int id, string name, double price, int quantity) {
        this->id = id;
        this->name = name;
        this->price = price;
        this->quantity = quantity;
        this->next = nullptr;
    }
};

// ================= Operation Stack for Undo =================
struct Operation {
    string type;  // "ADD", "UPDATE", "DELETE"
    int id;
    string name;
    double price;
    int quantity;
};

class OperationStack {
    struct Node {
        Operation op;
        Node* next;
    };
    Node* top;

public:
    OperationStack() { top = nullptr; }

    void push(Operation op) {
        Node* newNode = new Node{op, top};
        top = newNode;
    }

    bool pop(Operation &op) {
        if (!top) return false;
        Node* temp = top;
        op = top->op;
        top = top->next;
        delete temp;
        return true;
    }

    bool isEmpty() { return top == nullptr; }
};

// ================= Inventory Management using SLL =================
class InventorySLL {
    ProductNode* head;
    OperationStack& undoStack; // reference to global stack

public:
    InventorySLL(OperationStack& stack) : head(nullptr), undoStack(stack) {}

    // Add Product
    void addProduct(int id, string name, double price, int quantity) {
        ProductNode* newNode = new ProductNode(id, name, price, quantity);
        if (!head) {
            head = newNode;
        } else {
            ProductNode* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }

        // push to stack for undo
        undoStack.push({"ADD", id, name, price, quantity});
    }

    // Update Product
    void updateProduct(int id, int quantity, double price) {
        ProductNode* temp = head;
        while (temp) {
            if (temp->id == id) {
                // store old values for undo
                undoStack.push({"UPDATE", id, temp->name, temp->price, temp->quantity});
                temp->quantity = quantity;
                temp->price = price;
                cout << "Product updated successfully.\n";
                return;
            }
            temp = temp->next;
        }
        cout << "Product not found.\n";
    }

    // Delete Product
    void deleteProduct(int id) {
        if (!head) return;
        if (head->id == id) {
            ProductNode* toDelete = head;
            head = head->next;
            // push deleted product to stack for undo
            undoStack.push({"DELETE", toDelete->id, toDelete->name, toDelete->price, toDelete->quantity});
            delete toDelete;
            cout << "Product deleted successfully.\n";
            return;
        }
        ProductNode* temp = head;
        while (temp->next && temp->next->id != id) temp = temp->next;
        if (temp->next) {
            ProductNode* toDelete = temp->next;
            undoStack.push({"DELETE", toDelete->id, toDelete->name, toDelete->price, toDelete->quantity});
            temp->next = temp->next->next;
            delete toDelete;
            cout << "Product deleted successfully.\n";
        }
    }

    // Display Available Products
    void displayProducts() {
        ProductNode* temp = head;
        cout << "\n--- Available Products ---\n";
        while (temp) {
            if (temp->quantity > 0) {
                cout << "ID: " << temp->id
                     << " | Name: " << temp->name
                     << " | Price: " << temp->price
                     << " | Quantity: " << temp->quantity << "\n";
            }
            temp = temp->next;
        }
    }

    // Display Out of Stock
    void displayOutOfStock() {
        ProductNode* temp = head;
        cout << "\n--- Out of Stock Products ---\n";
        bool found = false;
        while (temp) {
            if (temp->quantity == 0) {
                cout << "ID: " << temp->id
                     << " | Name: " << temp->name << "\n";
                found = true;
            }
            temp = temp->next;
        }
        if (!found) cout << "No products are out of stock.\n";
    }

    // Reduce stock on order
    bool reduceStock(int id) {
        ProductNode* temp = head;
        while (temp) {
            if (temp->id == id && temp->quantity > 0) {
                temp->quantity -= 1;
                return true;
            }
            temp = temp->next;
        }
        return false;
    }

    // Undo last operation
    void undoLastOperation() {
        Operation op;
        if (!undoStack.pop(op)) {
            cout << "No operations to undo.\n";
            return;
        }

        if (op.type == "ADD") {
            // remove last added product
            deleteProduct(op.id);
            cout << "Undo: Added product removed.\n";
        } else if (op.type == "UPDATE") {
            // restore old values
            ProductNode* temp = head;
            while (temp) {
                if (temp->id == op.id) {
                    temp->price = op.price;
                    temp->quantity = op.quantity;
                    cout << "Undo: Product restored to previous state.\n";
                    return;
                }
                temp = temp->next;
            }
        } else if (op.type == "DELETE") {
            // re-add deleted product
            addProduct(op.id, op.name, op.price, op.quantity);
            cout << "Undo: Deleted product restored.\n";
        }
    }
};

// ================= Order Queue (FIFO) =================
class OrderQueue {
    struct Order {
        int productId;
        Order* next;
        Order(int id) { productId = id; next = nullptr; }
    };

    Order* front;
    Order* rear;

public:
    OrderQueue() { front = rear = nullptr; }

    void enqueue(int productId) {
        Order* newOrder = new Order(productId);
        if (!rear) {
            front = rear = newOrder;
        } else {
            rear->next = newOrder;
            rear = newOrder;
        }
        cout << "Order placed for product ID: " << productId << "\n";
    }

    void dequeue() {
        if (!front) {
            cout << "No orders to process.\n";
            return;
        }
        Order* temp = front;
        cout << "Processing order for product ID: " << temp->productId << "\n";
        front = front->next;
        if (!front) rear = nullptr;
        delete temp;
    }

    void displayOrders() {
        if (!front) {
            cout << "No pending orders.\n";
            return;
        }
        cout << "\n--- Pending Orders ---\n";
        Order* temp = front;
        while (temp) {
            cout << "Product ID: " << temp->productId << "\n";
            temp = temp->next;
        }
    }
};

// ================= Main E-Commerce System =================
class ECommerceSystem {
    OperationStack undoStack;
    InventorySLL inventory;
    OrderQueue orders;

public:
    ECommerceSystem() : inventory(undoStack) {}

    void adminPanel() {
        int choice;
        do {
            cout << "\n--- Admin Panel ---\n";
            cout << "1. Add Product\n2. Update Product\n3. Delete Product\n4. View Products\n5. View Out of Stock\n6. Undo Last Operation\n7. Back\n";
            cout << "Enter choice: ";
            cin >> choice;

            if (choice == 1) {
                int id, qty;
                string name;
                double price;
                cout << "Enter ID Name Price Quantity: ";
                cin >> id >> name >> price >> qty;
                inventory.addProduct(id, name, price, qty);
            } else if (choice == 2) {
                int id, qty;
                double price;
                cout << "Enter ID NewPrice NewQuantity: ";
                cin >> id >> price >> qty;
                inventory.updateProduct(id, qty, price);
            } else if (choice == 3) {
                int id;
                cout << "Enter ID to delete: ";
                cin >> id;
                inventory.deleteProduct(id);
            } else if (choice == 4) {
                inventory.displayProducts();
            } else if (choice == 5) {
                inventory.displayOutOfStock();
            } else if (choice == 6) {
                inventory.undoLastOperation();
            }

        } while (choice != 7);
    }

    void customerPanel() {
        int choice;
        do {
            cout << "\n--- Customer Panel ---\n";
            cout << "1. View Products\n2. Place Order\n3. View Orders\n4. Back\n";
            cout << "Enter choice: ";
            cin >> choice;

            if (choice == 1) {
                inventory.displayProducts();
            } else if (choice == 2) {
                int id;
                cout << "Enter Product ID to order: ";
                cin >> id;
                if (inventory.reduceStock(id)) {
                    orders.enqueue(id);
                } else {
                    cout << "Product not available or out of stock.\n";
                }
            } else if (choice == 3) {
                orders.displayOrders();
            }

        } while (choice != 4);
    }

    void mainMenu() {
        int choice;
        do {
            cout << "\n=== E-Commerce Inventory System ===\n";
            cout << "1. Admin Panel\n2. Customer Panel\n3. Process Order\n4. Exit\n";
            cout << "Enter choice: ";
            cin >> choice;

            if (choice == 1) adminPanel();
            else if (choice == 2) customerPanel();
            else if (choice == 3) orders.dequeue();

        } while (choice != 4);
    }
};

// ================= Main =================
int main() {
    ECommerceSystem system;
    system.mainMenu();
    return 0;
}
