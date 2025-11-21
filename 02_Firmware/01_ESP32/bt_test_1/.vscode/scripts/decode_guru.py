#!/usr/bin/env python3
import sys
import os
import subprocess
import glob

def find_elf_file(elf_path):
    """Find and validate ELF file path"""
    if not os.path.isfile(elf_path):
        # Try to find in common locations
        possible_paths = [
            elf_path,
            f"build/{elf_path}",
            f"{elf_path}.elf",
            f"build/{elf_path}.elf",
            "build/project.elf",
            "build/*.elf"
        ]
        
        for path in possible_paths:
            if "*" in path:
                # Handle glob patterns
                matches = glob.glob(path)
                if matches:
                    return os.path.abspath(matches[0])
            elif os.path.isfile(path):
                return os.path.abspath(path)
        
        print(f"❌ ELF file not found: {elf_path}")
        print("Searched in:")
        for path in possible_paths:
            print(f"  - {path}")
        return None
    return os.path.abspath(elf_path)

def get_addr2line_path():
    """Get addr2line path from environment variable"""
    addr2line_path = os.environ.get('ADDR2LINE_PATH')
    
    if not addr2line_path:
        print("❌ ADDR2LINE_PATH environment variable not set!")
        print("Please set ADDR2LINE_PATH in tasks.json")
        return None
    
    if not os.path.isfile(addr2line_path):
        print(f"❌ addr2line not found at: {addr2line_path}")
        print("Please update ADDR2LINE_PATH in tasks.json with correct path")
        return None
    
    print(f"✅ Using addr2line: {addr2line_path}")
    return addr2line_path

def decode_addresses(elf_file, addresses):
    """Decode addresses using addr2line"""
    addr2line = get_addr2line_path()
    
    if not addr2line:
        return False
    
    if not os.path.isfile(elf_file):
        print(f"❌ ELF file not found: {elf_file}")
        return False
    
    print(f"📁 ELF file: {elf_file}")
    print(f"🔍 Decoding addresses: {', '.join(addresses)}")
    print("=" * 60)
    
    # Prepare command
    cmd = [addr2line, "-p", "-a", "-f", "-s", "-C", "-i", "-e", elf_file] + addresses
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode == 0:
            print("✅ Decoded stack trace:")
            print(result.stdout)
        else:
            print("❌ Error during decoding:")
            print(result.stderr)
            return False
    except Exception as e:
        print(f"❌ Failed to run addr2line: {e}")
        return False
    
    return True

def extract_addresses(user_input):
    """Extract addresses from various input formats"""
    addresses = []
    
    # Remove common prefixes and split
    clean_input = user_input.replace("Backtrace:", "").strip()
    
    for part in clean_input.split():
        # Handle both 0x400d161d:0x3ffb1ae0 and 0x400d161d formats
        if ':' in part:
            addr = part.split(':')[0]
        else:
            addr = part
        
        # Ensure it starts with 0x
        if addr.startswith('0x'):
            addresses.append(addr)
        elif len(addr) > 0:
            # Assume it's hex without 0x prefix
            addresses.append(f"0x{addr}")
    
    return addresses

def main():
    if len(sys.argv) < 2:
        print("Usage: python decode_guru.py <elf_file>")
        sys.exit(1)
    
    elf_file = sys.argv[1]
    elf_path = find_elf_file(elf_file)
    
    if not elf_path:
        print("\n💡 Tip: Compile your project first with 'idf.py build'")
        sys.exit(1)
    
    print("🧠 ESP32 Guru Meditation Decoder")
    print("=" * 50)
    print("📝 Paste your backtrace (examples):")
    print("   • 0x400d161d:0x3ffb1ae0 0x400d1c5d:0x3ffb1b00")
    print("   • Backtrace: 0x400d161d:0x3ffb1ae0 0x400d1c5d:0x3ffb1b00")
    print("   • 400d161d 400d1c5d")
    print("Type 'exit' to quit")
    print("-" * 50)
    
    while True:
        try:
            user_input = input("\n📥 Enter backtrace addresses: ").strip()
        except KeyboardInterrupt:
            print("\n👋 Goodbye!")
            break
        
        if user_input.lower() in ['exit', 'quit', '']:
            break
        
        addresses = extract_addresses(user_input)
        
        if not addresses:
            print("❌ No valid addresses found in input")
            print("💡 Try: 0x400d161d 0x400d1c5d")
            continue
        
        success = decode_addresses(elf_path, addresses)
        
        if not success:
            print("❌ Decoding failed.")

if __name__ == "__main__":
    main()