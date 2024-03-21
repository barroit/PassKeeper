# PassKeeper - A password manager

![CMake](https://img.shields.io/badge/cmake-0D1117?style=for-the-badge&logo=cmake&logoColor=064F8C)&nbsp;
![VSCode](https://img.shields.io/badge/vscode-0D1117?style=for-the-badge&logo=visualstudiocode&logoColor=007ACC)&nbsp;
![Code Size](https://img.shields.io/github/languages/code-size/barroit/PassKeeper?style=for-the-badge&labelColor=0D1117&color=0D1117)&nbsp;

## Build

You can build PassKeeper by running `./brtool`, some of commands are available:

+ build
+ test
+ clean

for more information, see `./brtool help`

## Usage

`pk <command> [<args>...]`

example:

+ pk help: show help messages
+ pk init: create a db file
+ pk read: read a record

## License

PassKeeper is licensed under the GNU General Public License v3.0 - see the COPYING file or visit [the text version of the GPL](https://www.gnu.org/licenses/gpl.txt) for details.

## Third-Party Licenses

This project uses several third-party components, each with its own licensing:

- **SQLCipher**: SQLCipher is an open source extension to SQLite that provides transparent 256-bit AES encryption of database files. For the licensing details of SQLCipher, please visit their [official website](https://www.zetetic.net/sqlcipher/license/) or see the *SQLCIPHER_LICENSE* file included in this project.

## Troubleshooting

OpenSSL assembly directives not being recognized: Ensure you have nasm installed

