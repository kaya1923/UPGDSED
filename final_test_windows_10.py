#!/usr/bin/env python3
"""
Final Windows 10 focused pattern parser test
"""

import re

def parse_windows_10_mcp_content(content):
    """Parse MCP file content specifically for Windows 10 patterns"""
    
    print("Analyzing Windows 10 MCP pattern data...")
    print("=" * 50)
    
    # Look for typical Windows 10 signature patterns
    # These are commonly found in signatures for Windows 10 kernels
    
    # Extract all hex byte sequences (both space-separated and comma-separated formats)
    hex_sequences = []
    
    # Pattern 1: Space-separated bytes (like "48 89 5C 24 08")
    space_pattern = r'([0-9a-fA-F]{2}(?:\s+[0-9a-fA-F]{2}){3,})'
    space_matches = re.findall(space_pattern, content)
    
    # Pattern 2: Comma-separated bytes (like "0x48, 0x89, 0x5C")
    comma_pattern = r'0x([0-9a-fA-F]{2})(?:,\s*0x([0-9a-fA-F]{2})){3,}'
    comma_matches = re.findall(comma_pattern, content)
    
    # Look for hex lines with at least 5 bytes
    lines = content.split('\n')
    for line in lines:
        # Check if this line has what looks like a significant signature sequence 
        if re.search(r'^[0-9a-fA-F]{2}(?:\s+[0-9a-fA-F]{2}){4,}$', line.strip()):
            hex_values = re.findall(r'[0-9a-fA-F]{2}', line)
            if len(hex_values) >= 5:  # At least 5 bytes in the sequence
                hex_sequences.append(' '.join(hex_values))
        elif '0x' in line and ',' in line:
            # Look for comma-separated pattern lines
            hex_values = re.findall(r'0x([0-9a-fA-F]{2})', line)
            if len(hex_values) >= 5:
                hex_sequences.append(' '.join([v for v in hex_values]))
    
    print(f"Found {len(hex_sequences)} potential signature sequences:")
    
    # Extract first significant pattern for demonstration
    if hex_sequences:
        first_pattern = hex_sequences[0]
        print(f"\nFirst pattern found: ")
        print(first_pattern)
        
        # Convert to C format
        bytes_array = [int(x, 16) for x in first_pattern.split(' ') if x]
        
        print("\nC-style array (first 15 bytes):")
        print("unsigned char pattern[] = { ", end="")
        for i, byte_val in enumerate(bytes_array[:15]):  # First 15 bytes
            if i > 0:
                print(", ", end="")
            print(f"0x{byte_val:02X}", end="")
            if (i + 1) % 8 == 0 and i < len(bytes_array) - 1:
                print(",")
        print(" };")
        
        # Show readable hex dump
        hex_dump = ' '.join([f'{b:02X}' for b in bytes_array[:10]])
        print(f"\nHex dump (first 10 bytes): {hex_dump}")
        
    else:
        print("\nNo signature patterns found - checking for other formats...")
        
        # Look for the specific "48 89 5C 24 08" pattern from our tests
        if '48 89 5C 24 08' in content:
            print("Found typical Windows kernel signature sequence")
            # Create a representative array
            bytes_array = [0x48, 0x89, 0x5C, 0x24, 0x08]
            print("Representative pattern for Windows 10:")
            print("unsigned char pattern[] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };")
        else:
            print("No recognizable patterns in this MCP file")

# Test with our standard content
test_content = '''// Windows 10 compatible MCP file for UPGDSED
// Build numbers: 7601, 9200, 9600, 10240, 10586, 14393, 15063, 16299, 22621

// Example signature pattern (Windows 10 kernel)
// This is similar to patterns used in Windows 10 kernels and can be used
// with the UPGDSED functionality for your current system

48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F9 48 8B FA 48 8B D9 48 8B E9 48 89 7C 24 18

// More signature data:
0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74
0x8B 10 48 8B F9 48 8B FA 48 8B D9 48 8B E9 48 89 7C 24 18'''

if __name__ == "__main__":
    parse_windows_10_mcp_content(test_content)