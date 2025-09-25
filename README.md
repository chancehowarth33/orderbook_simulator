# Simple Limit Order Book in C++  

This project implements a **limit order book and matching engine** in modern C++. It simulates how financial exchanges (NASDAQ, CME, crypto exchanges, etc.) manage and match buy/sell orders in real-time.  

It’s designed as a learning tool for **systems programming, finance, and high-frequency trading concepts**, but structured to be easily extended for performance benchmarking.  

---

## ✨ Features  
- **Buy/Sell limit orders** with unique IDs  
- **Order matching** at the resting order’s price  
- **Partial fills** with leftover quantity resting in the book  
- **FIFO (first-in, first-out)** order handling at each price level  
- **Order cancelation** by ID  
- **REPL interface** for interactive testing  
- Easy to extend for **market orders, logging, or latency benchmarks**  

---

## ⚙️ Build Instructions  

```bash
# clone the repo and compile the code with c++17 or later
g++ -O2 -std=c++17 -Wall -Wextra -o order_book order_book.cpp

# run the repl
./order_book

```bash

## 📖 Example Session  

```bash
$ ./order_book
simple order book repl
commands: BUY px qty | SELL px qty | CANCEL id | PRINT | TOP | QUIT

BUY 100 50
order id=1 accepted

SELL 101 20
order id=2 accepted

TOP
---- top of book ----
ask: 101 x 20
bid: 100 x 50
---------------------

BUY 101 30
order id=3 accepted
trade: price=101 qty=20 (buy_id=3, sell_id=2)

PRINT

=========== order book ===========
   -- asks (low -> high) --
  (empty)
   -- bids (high -> low) --
       101  |  qty=10 (orders=1)
       100  |  qty=50 (orders=1)
==================================


