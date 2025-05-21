<a id="readme-top"></a>

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![project_license][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/AdamMinge/specter">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">Specter</h3>

  <p align="center">
    Specter is a dynamic, injectable C++/Qt DLL that embeds a gRPC server into any Qt-based application, providing structured runtime access to its internal object model. It allows for non-invasive querying, manipulation, and automation of UI elements.
    <br />
    <a href="https://github.com/AdamMinge/specter"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/AdamMinge/specter">View Demo</a>
    &middot;
    <a href="https://github.com/AdamMinge/specter/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
    &middot;
    <a href="https://github.com/AdamMinge/specter/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project

Specter is a modular runtime introspection layer for Qt applications. Once injected into a target process, it initializes a self-contained gRPC server that exposes the internal object graph of the host application to external clients.

Built for test automation, debugging, and deep runtime inspection, Specter provides capabilities such as:

- Hierarchical UI object discovery
- Real-time property inspection and mutation
- Invocation of QObject methods and slots
- High-level simulation of user interactions
- Recording of interaction sequences for automation

Specter operates entirely out-of-process, requiring no modification to the host application’s codebase. It is suitable for both internal tooling and robust automation frameworks, and acts as the foundation for remote control and instrumentation of Qt-based systems.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Built With

* [![cpp][cpp]][cpp-url]
* [![cmake][cmake]][cmake-url]
* [![qt6][qt6]][qt6-url]
* [![grpc][grpc]][grpc-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

This guide walks you through setting up and building the project locally using CMake. Follow the steps below to configure, build, and run the project.

### Prerequisites

Make sure the following tools are installed on your system:

- CMake (version 3.20+ recommended)
- Git
- A C++ Compiler (e.g., MSVC, Clang, or GCC)

Example (installing CMake with a package manager on Linux):
```sh
sudo apt install cmake
```
Or on Windows, install via https://cmake.org/download/

### Installation

#### 1. Clone the repository

```sh
git clone https://github.com/AdamMinge/specter.git
cd specter
```

#### 2. Configure the project with CMake

This step will:

- Download and build required dependencies: Qt and gRPC
- Set up the build system to compile the project

```sh
cmake -S . -B build
```

#### 3. Build the project

This will compile all necessary code and generate the specter.dll (or corresponding platform-specific shared library)

```sh
cmake --build build/ --config Debug --target all -j 18 --
```

#### Notes
- All dependencies are handled automatically through the CMake build system. You do not need to manually install Qt or gRPC.

- If you run into build issues, verify that your compiler and CMake version meet minimum requirements and are compatible with Qt and gRPC.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

- [ ] Support for listen on property change 
- [ ] Extended recorder

See the [open issues](https://github.com/AdamMinge/specter/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Top contributors:

<a href="https://github.com/AdamMinge/specter/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=AdamMinge/specter" alt="contrib.rocks image" />
</a>

<!-- LICENSE -->
## License

Distributed under the Unlicense License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- CONTACT -->
## Contact

Adam Minge - minge.adam@gmail.com

Project Link: [https://github.com/AdamMinge/specter](https://github.com/AdamMinge/specter)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/AdamMinge/specter.svg?style=for-the-badge
[contributors-url]: https://github.com/AdamMinge/specter/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/AdamMinge/specter.svg?style=for-the-badge
[forks-url]: https://github.com/AdamMinge/specter/network/members
[stars-shield]: https://img.shields.io/github/stars/AdamMinge/specter.svg?style=for-the-badge
[stars-url]: https://github.com/AdamMinge/specter/stargazers
[issues-shield]: https://img.shields.io/github/issues/AdamMinge/specter.svg?style=for-the-badge
[issues-url]: https://github.com/AdamMinge/specter/issues
[license-shield]: https://img.shields.io/github/license/AdamMinge/specter.svg?style=for-the-badge
[license-url]: https://github.com/AdamMinge/specter/blob/master/LICENSE.txt
[cpp]: https://img.shields.io/badge/C++-20+-00599C?logo=c%2b%2b&logoColor=white
[cpp-url]: https://isocpp.org/
[cmake]: https://img.shields.io/badge/CMake-3.20+-064F8C?logo=cmake&logoColor=white
[cmake-url]: https://cmake.org/
[qt6]: https://img.shields.io/badge/Qt-6.9-green?logo=qt&logoColor=white
[qt6-url]: https://www.qt.io/
[grpc]: https://img.shields.io/badge/gRPC-1.66.1-brightgreen?logo=grpc&logoColor=white
[grpc-url]: https://grpc.io/