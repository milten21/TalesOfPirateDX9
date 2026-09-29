"""
AccountManager.py - PostgreSQL Account Manager GUI

How to use:

- Requirements:
    - Python 3.8+
    - `psycopg2` package (PostgreSQL client library)

- Install dependencies (run in Command Prompt):

        pip install psycopg2-binary

- Run the app from the command line:

        cd postgresql
        py -3 AccountManager.py

    or, if `py` launcher is not available:

        python AccountManager.py

- Notes:
    - The role you connect with needs access to both the AccountServer and GameDB databases
      (for example `postgres`); the service roles pko_account / pko_game each own only one.
    - If you see Tcl/Tk errors, ensure your Python installation includes `tkinter` (standard on Windows installers).

"""

import tkinter as tk
from tkinter import messagebox, ttk
import hashlib
import binascii
import psycopg2

ACCOUNT_DB = "AccountServer"
GAME_DB = "GameDB"


def blake2s_hash(password):
    hasher = hashlib.blake2s()
    hasher.update(password.encode('utf-8'))
    digest = hasher.digest()
    return binascii.hexlify(digest).decode('utf-8').upper()


def connect(host, port, dbname, user, password):
    conn = psycopg2.connect(host=host, port=port, dbname=dbname, user=user, password=password)
    conn.autocommit = True
    return conn


def account_exists(conn, acc_name):
    with conn.cursor() as cur:
        cur.execute("SELECT name FROM account_login WHERE name = %s", (acc_name,))
        return cur.fetchone() is not None


def create_account_login(conn, acc_name, hash_pwd):
    """AccountServer half: the login the AccountServer authenticates against."""
    with conn.cursor() as cur:
        cur.execute("INSERT INTO account_login (name, password) VALUES (%s, %s)", (acc_name, hash_pwd))


def create_game_account(conn, acc_name):
    """GameDB half: the account row the GameServer/GroupServer use."""
    with conn.cursor() as cur:
        cur.execute("INSERT INTO account (ato_id, ato_nome, jmes) "
                    "VALUES ((SELECT COALESCE(MAX(ato_id) + 1,1) FROM account), %s, 99)", (acc_name,))

class AccountManagerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("PostgreSQL Account Manager")
        self.root.geometry("450x550")
        self.root.resizable(False, False)

        # Style
        self.style = ttk.Style()
        self.style.configure("TLabel", font=("Segoe UI", 10))
        self.style.configure("TButton", font=("Segoe UI", 10, "bold"))
        self.style.configure("Header.TLabel", font=("Segoe UI", 12, "bold"))

        # Main Container
        main_frame = ttk.Frame(root, padding="20")
        main_frame.pack(fill=tk.BOTH, expand=True)

        # SQL Connection Section
        ttk.Label(main_frame, text="Database Connection Settings", style="Header.TLabel").grid(row=0, column=0, columnspan=2, pady=(0, 15), sticky=tk.W)

        ttk.Label(main_frame, text="Server:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.server_entry = ttk.Entry(main_frame, width=30)
        self.server_entry.insert(0, "127.0.0.1")
        self.server_entry.grid(row=1, column=1, pady=5)

        ttk.Label(main_frame, text="Port:").grid(row=2, column=0, sticky=tk.W, pady=5)
        self.port_entry = ttk.Entry(main_frame, width=30)
        self.port_entry.insert(0, "5432")
        self.port_entry.grid(row=2, column=1, pady=5)

        self.user_label = ttk.Label(main_frame, text="DB Username:")
        self.user_label.grid(row=3, column=0, sticky=tk.W, pady=5)
        self.user_entry = ttk.Entry(main_frame, width=30)
        self.user_entry.insert(0, "postgres")
        self.user_entry.grid(row=3, column=1, pady=5)

        self.pwd_label = ttk.Label(main_frame, text="DB Password:")
        self.pwd_label.grid(row=4, column=0, sticky=tk.W, pady=5)
        self.pwd_entry = ttk.Entry(main_frame, width=30, show="*")
        self.pwd_entry.insert(0, "passwordhere")
        self.pwd_entry.grid(row=4, column=1, pady=5)

        ttk.Separator(main_frame, orient=tk.HORIZONTAL).grid(row=5, column=0, columnspan=2, sticky=tk.EW, pady=20)

        # Account Creation Section
        ttk.Label(main_frame, text="Create New Account", style="Header.TLabel").grid(row=6, column=0, columnspan=2, pady=(0, 15), sticky=tk.W)

        ttk.Label(main_frame, text="Account Name:").grid(row=7, column=0, sticky=tk.W, pady=5)
        self.acc_name_entry = ttk.Entry(main_frame, width=30)
        self.acc_name_entry.grid(row=7, column=1, pady=5)

        ttk.Label(main_frame, text="Account Password:").grid(row=8, column=0, sticky=tk.W, pady=5)
        self.acc_pwd_entry = ttk.Entry(main_frame, width=30, show="*")
        self.acc_pwd_entry.grid(row=8, column=1, pady=5)

        # Limit account name length
        self.acc_name_entry.bind("<KeyRelease>", self.validate_length)

        # Create Button
        self.create_btn = ttk.Button(main_frame, text="CREATE ACCOUNT", command=self.create_account)
        self.create_btn.grid(row=9, column=0, columnspan=2, pady=30, ipady=5, sticky=tk.EW)

        # Status Bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(main_frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W, font=("Segoe UI", 9))
        status_bar.grid(row=10, column=0, columnspan=2, sticky=tk.EW, pady=(10, 0))

    def validate_length(self, event):
        content = self.acc_name_entry.get()
        if len(content) > 16:
            self.acc_name_entry.delete(16, tk.END)
            self.status_var.set("Warning: Max length 16 characters reached")
        else:
            self.status_var.set("Ready")

    def create_account(self):
        acc_name = self.acc_name_entry.get().strip()
        acc_pwd = self.acc_pwd_entry.get().strip()

        if not acc_name or not acc_pwd:
            messagebox.showerror("Error", "Please enter both account name and password.")
            return

        if len(acc_name) > 16:
            messagebox.showerror("Error", "Account name cannot be longer than 16 characters.")
            return

        hash_pwd = blake2s_hash(acc_pwd)
        server = self.server_entry.get()
        user = self.user_entry.get()
        pwd = self.pwd_entry.get()

        try:
            port = int(self.port_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Port must be a number.")
            return

        conn = None
        try:
            self.status_var.set("Connecting to database...")
            self.root.update_idletasks()
            conn = connect(server, port, ACCOUNT_DB, user, pwd)

            # Check if account exists
            if account_exists(conn, acc_name):
                messagebox.showerror("Error", f"Account '{acc_name}' already exists!")
                return

            # Insert into AccountServer
            create_account_login(conn, acc_name, hash_pwd)
            conn.close()

            # Insert into GameDB
            conn = connect(server, port, GAME_DB, user, pwd)
            create_game_account(conn, acc_name)

            messagebox.showinfo("Success", f"Account '{acc_name}' created successfully!")
            self.status_var.set(f"Successfully created account: {acc_name}")

            # Clear fields
            self.acc_name_entry.delete(0, tk.END)
            self.acc_pwd_entry.delete(0, tk.END)

        except Exception as e:
            messagebox.showerror("Database Error", str(e))
            self.status_var.set("Error occurred during creation.")
        finally:
            if conn is not None and not conn.closed:
                conn.close()

if __name__ == "__main__":
    root = tk.Tk()
    app = AccountManagerApp(root)
    root.mainloop()
