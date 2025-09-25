#include <bits/stdc++.h>
using namespace std;

using OrderID   = uint64_t;
using Quantity  = uint64_t;
using Price     = int64_t;   // price in integer ticks (e.g., cents) //

enum class Side : uint8_t { BUY = 0, SELL = 1 };

struct Trade {
    OrderID buy_id;
    OrderID sell_id;
    Price   price;     // executed price (resting order's price) //
    Quantity qty;
};

struct Order {
    OrderID id;
    Side    side;
    Price   price;
    Quantity qty;      // remaining quantity //
    uint64_t seq;      // fifo tiebreaker //
};

using LevelQueue = deque<Order>;   // a price level holds fifo queue of resting orders //

struct Desc {
    bool operator()(const Price& a, const Price& b) const { return a > b; } // for bids: sort descending //
};

class OrderBook {
public:
    OrderBook() : next_id_(1), seq_(0) {}

    // add a limit order, return its id and any generated trades //
    pair<OrderID, vector<Trade>> add_limit(Side side, Price px, Quantity qty) {
        vector<Trade> trades;
        if (qty == 0) return {0, trades};

        OrderID incoming_id = next_id_;
        if (side == Side::BUY) {
            match_buy(px, qty, trades, incoming_id);
            if (qty > 0) add_resting(side, px, qty, incoming_id);
        } else {
            match_sell(px, qty, trades, incoming_id);
            if (qty > 0) add_resting(side, px, qty, incoming_id);
        }
        return {incoming_id, trades};
    }

    // cancel by order id //
    bool cancel(OrderID id) {
        auto it = index_.find(id);
        if (it == index_.end()) return false;
        const auto& info = it->second;
        Side side = info.side;
        Price px = info.price;

        auto& book = (side == Side::BUY) ? bids_ : asks_;
        auto lvl = book.find(px);
        if (lvl == book.end()) { index_.erase(it); return false; }

        auto& q = lvl->second;
        bool removed = false;
        for (auto dqit = q.begin(); dqit != q.end(); ++dqit) {
            if (dqit->id == id) {
                q.erase(dqit);
                removed = true;
                break;
            }
        }
        if (q.empty()) book.erase(lvl);
        index_.erase(it);
        return removed;
    }

    // print top of book //
    void print_top() const {
        cout << "---- top of book ----\n";
        if (!asks_.empty()) {
            auto it = asks_.begin();
            cout << "ask: " << it->first << " x " << level_qty(it->second) << "\n";
        } else {
            cout << "ask: (empty)\n";
        }
        if (!bids_.empty()) {
            auto it = bids_.begin();
            cout << "bid: " << it->first << " x " << level_qty(it->second) << "\n";
        } else {
            cout << "bid: (empty)\n";
        }
        cout << "---------------------\n";
    }

    // print book snapshot //
    void print_book(size_t depth = 5) const {
        cout << "\n=========== order book ===========\n";
        cout << "   -- asks (low -> high) --\n";
        size_t shown = 0;
        for (auto it = asks_.begin(); it != asks_.end() && shown < depth; ++it, ++shown) {
            cout << "  " << setw(8) << it->first << "  |  qty=" << level_qty(it->second) 
                 << " (orders=" << it->second.size() << ")\n";
        }
        if (shown == 0) cout << "  (empty)\n";

        cout << "   -- bids (high -> low) --\n";
        shown = 0;
        for (auto it = bids_.begin(); it != bids_.end() && shown < depth; ++it, ++shown) {
            cout << "  " << setw(8) << it->first << "  |  qty=" << level_qty(it->second) 
                 << " (orders=" << it->second.size() << ")\n";
        }
        if (shown == 0) cout << "  (empty)\n";
        cout << "==================================\n\n";
    }

private:
    // match logic for buys //
    void match_buy(Price limit_px, Quantity& qty, vector<Trade>& out, OrderID incoming_id) {
        while (qty > 0 && !asks_.empty()) {
            auto best = asks_.begin();
            Price best_px = best->first;
            if (best_px > limit_px) break;

            auto& q = best->second;
            while (qty > 0 && !q.empty()) {
                Order& resting = q.front();
                Quantity fill = min(qty, resting.qty);
                out.push_back(Trade{ incoming_id, resting.id, best_px, fill });

                qty         -= fill;
                resting.qty -= fill;

                if (resting.qty == 0) {
                    index_.erase(resting.id);
                    q.pop_front();
                }
            }
            if (q.empty()) asks_.erase(best);
        }
    }

    // match logic for sells //
    void match_sell(Price limit_px, Quantity& qty, vector<Trade>& out, OrderID incoming_id) {
        while (qty > 0 && !bids_.empty()) {
            auto best = bids_.begin();
            Price best_px = best->first;
            if (best_px < limit_px) break;

            auto& q = best->second;
            while (qty > 0 && !q.empty()) {
                Order& resting = q.front();
                Quantity fill = min(qty, resting.qty);
                out.push_back(Trade{ resting.id, incoming_id, best_px, fill });

                qty         -= fill;
                resting.qty -= fill;

                if (resting.qty == 0) {
                    index_.erase(resting.id);
                    q.pop_front();
                }
            }
            if (q.empty()) bids_.erase(best);
        }
    }

    // add remainder as resting order //
    void add_resting(Side side, Price px, Quantity qty, OrderID id) {
        Order o;
        o.id  = id;
        o.side = side;
        o.price = px;
        o.qty = qty;
        o.seq = ++seq_;

        if (side == Side::BUY) {
            bids_[px].push_back(o);
        } else {
            asks_[px].push_back(o);
        }
        index_[o.id] = { side, px };
        next_id_++;
    }

    struct IndexInfo {
        Side  side;
        Price price;
    };

    static Quantity level_qty(const LevelQueue& q) {
        Quantity total = 0;
        for (auto& o : q) total += o.qty;
        return total;
    }

private:
    map<Price, LevelQueue> asks_;             // ascending //
    map<Price, LevelQueue, Desc> bids_;       // descending //
    unordered_map<OrderID, IndexInfo> index_; // id -> side, price //
    OrderID  next_id_;
    uint64_t seq_;
};

// ------------------------------- repl ----------------------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    OrderBook ob;

    cout << "simple order book repl\n";
    cout << "commands: BUY px qty | SELL px qty | CANCEL id | PRINT | TOP | QUIT\n";

    string cmd;
    while (cin >> cmd) {
        if (cmd == "BUY" || cmd == "SELL") {
            Price px; Quantity qty;
            cin >> px >> qty;
            Side side = (cmd == "BUY") ? Side::BUY : Side::SELL;
            auto [id, trades] = ob.add_limit(side, px, qty);
            cout << "order id=" << id << " accepted\n";
            for (auto& t : trades) {
                cout << "trade: price=" << t.price << " qty=" << t.qty
                     << " (buy_id=" << t.buy_id << ", sell_id=" << t.sell_id << ")\n";
            }
        } else if (cmd == "CANCEL") {
            OrderID id; cin >> id;
            bool ok = ob.cancel(id);
            cout << (ok ? "canceled\n" : "not found\n");
        } else if (cmd == "PRINT") {
            ob.print_book();
        } else if (cmd == "TOP") {
            ob.print_top();
        } else if (cmd == "QUIT") {
            break;
        } else {
            cout << "unknown command\n";
        }
    }
    return 0;
}
