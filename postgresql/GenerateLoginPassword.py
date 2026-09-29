import hashlib
import binascii
import sys

def blake2s_hash(password):
    # Create a BLAKE2s hash object
    hasher = hashlib.blake2s()

    # Update the hasher with the password (string should be encoded to bytes)
    hasher.update(password.encode('utf-8'))

    # Get the digest (hash result)
    digest = hasher.digest()

    # Convert the digest to a hexadecimal string
    hexencoded = binascii.hexlify(digest).decode('utf-8').upper()

    return hexencoded

if __name__ == "__main__":
    # Prints the hash to store in account_login.password. Without an argument: the hash of "admin".
    password = sys.argv[1] if len(sys.argv) > 1 else "admin"
    hexencoded = blake2s_hash(password)
    print(hexencoded)
