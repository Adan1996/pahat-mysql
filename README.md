# 📖 PAHATLang MySQL Module - Developer Documentation

Dokumentasi ini ditujukan bagi pengembang yang ingin berkontribusi, memelihara, atau mengembangkan lebih lanjut modul eksternal MySQL (`mysql_pahat`) untuk bahasa pemrogram PAHATLang.

---

## 🏗️ 1. Arsitektur Proyek

Modul ini dibangun menggunakan bahasa C sebagai *native binding* antara interpreter PAHATLang dan pustaka Klien MySQL (`libmysql`).

```
mysql-driver/
├── .github/
│   └── workflows/
│       └── release.yml     # Automasi Build & Release via GitHub Actions
├── include/
│   └── db_driver.h         # Header kontrak interface modul PAHATLang
├── lib/
│   └── libmysql.dll        # Shared library resmi MySQL (Windows)
├── modules/
│   ├── mysql_pahat.dll     # Output kompilasi Windows
│   └── mysql_pahat.so      # Output kompilasi Linux
├── src/
│   └── mysql_driver.c      # Implementasi native driver C
└── Makefile                # Script kompilasi lintas platform

```

---

## 🛠️ 2. Prasyarat Lingkungan Pengembangan (Development Prerequisites)

Untuk mengompilasi dan mengembangkan proyek ini di lingkungan lokal, Anda **tidak memerlukan MSYS2/MinGW64 manual**. Gunakan *toolchain* native standar berikut sesuai Sistem Operasi Anda:

### **A. Windows**

1. **GCC Compiler**: Pasang GCC native via [WinLibs](https://winlibs.com/?utm_source=gemini) atau installer MinGW-w64 standalone, lalu daftarkan folder `bin` ke System `PATH`.
2. **GNU Make**: Pasang utilitas `make` (bisa didapatkan melalui installer `make` untuk Windows atau via Chocolatey: `choco install make`).
3. **Pustaka Klien MySQL**: Pustaka `libmysql.dll` dan header MySQL Connector/C versi 6.1+ sudah disediakan di dalam folder `lib/` dan `include/` proyek.

### **B. Linux (Ubuntu/Debian)**

Install compiler dan pustaka pendukung via terminal:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libmysqlclient-dev

```

### **C. macOS**

Install Xcode Command Line Tools dan `mysql-client` via Homebrew:

```bash
xcode-select --install
brew install mysql-client

```

---

## ⚙️ 3. Proses Kompilasi Lokal

Kompilasi dilakukan menggunakan perintah `make` sederhana yang secara otomatis mendeteksi Sistem Operasi yang digunakan.

### **Menjalankan Kompilasi**

Buka terminal/Command Prompt di direktori utama proyek, lalu jalankan:

```bash
# Membersihkan hasil build sebelumnya
make clean

# Memulai proses kompilasi
make

```

Output berkas *binary* akan disimpan otomatis ke dalam direktori `modules/`:

* **Windows**: `modules/mysql_pahat.dll`
* **Linux**: `modules/mysql_pahat.so`
* **macOS**: `modules/mysql_pahat.dylib`

---

## 🔌 4. Antarmuka Native Module (C API Contract)

Setiap fungsi yang diekspos ke PAHATLang didefinisikan di dalam `src/mysql_driver.c` dan mengikuti kontrak antarmuka `db_driver.h`.

### **Pengembangan Fungsi Baru**

Jika Anda ingin menambahkan fitur baru (misalnya transaksi `BEGIN/COMMIT` atau *prepared statement*):

1. **Tambahkan deklarasi fungsi** pada header `include/db_driver.h`:
```c
// Contoh: Menambahkan fungsi transaksi
int pahat_mysql_begin_transaction(void* conn);

```


2. **Implementasikan logika C** di `src/mysql_driver.c`:
```c
#include <mysql.h>
#include "../include/db_driver.h"

int pahat_mysql_begin_transaction(void* conn) {
    MYSQL *mysql_conn = (MYSQL*)conn;
    if (!mysql_conn) return 0;

    return (mysql_query(mysql_conn, "START TRANSACTION") == 0);
}

```


3. **Registrasikan simbol fungsi** agar dapat dipanggil oleh interpreter PAHATLang saat modul di-*load* dinamis via `LoadLibrary` / `dlopen`.

---

## 🚀 5. Otomatisasi Rilis (CI/CD Pipeline)

Proyek ini dikonfigurasi menggunakan **GitHub Actions** untuk melakukan kompilasi otomatis (*cross-compile*) lintas platform setiap kali *tag* versi baru di-*push*.

### **Workflow Release (`.github/workflows/release.yml`)**

Ketika Anda memublikasikan rilis baru, GitHub Actions akan menjalankan alur berikut:

1. Membangun *binary* native di runner **Windows** dan **Linux**.
2. Mengambil `lib/libmysql.dll` untuk platform Windows.
3. Mengunggah seluruh aset ke dalam **GitHub Release** tag terkait.

### **Cara Memublikasikan Versi Baru**

Gunakan perintah `git tag` untuk memicu publikasi rilis otomatis:

```bash
# 1. Buat tag versi baru (misal: v1.1.0)
git tag v1.1.0

# 2. Push tag ke repository remote
git push origin v1.1.0

```

GitHub Actions akan memproses rilis tersebut, dan pengguna dapat langsung memperbarui modulnya melalui Package Manager PAHATLang (`pahat pm`):

```bash
pahat pm install mysql

```

---

## 🐛 6. Panduan Debugging & Troubleshooting

| Gejala Error | Penyebab Utama | Solusi Pengembang |
| --- | --- | --- |
| `ErrorCode 126` (Windows) | Berkas `libmysql.dll` tidak ditemukan di folder `modules/`. | Pastikan `libmysql.dll` diikutsertakan di folder `modules/` atau terinstal di `PATH` sistem. |
| `undefined reference to mysql_*` | Pustaka MySQL tidak terikat (*linked*) saat proses kompilasi. | Periksa variabel `LIBS` pada `Makefile` untuk memastikan `-lmysqlclient` atau path `libmysql.lib` valid. |
| `invalid header path` | Header `mysql.h` tidak ditemukan oleh compiler. | Pastikan lokasi `-Iinclude` pada `Makefile` mengarah ke folder header yang benar. |