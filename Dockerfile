FROM ubuntu:18.04 as sgext-base
MAINTAINER Pablo Hernandez Cerdan <pablo.hernandez.cerdan@outlook.com>

# Install basics
RUN apt-get update && \
    apt-get install -y \
    build-essential  wget  git \
    ninja-build \
    vim \
    freeglut3-dev \
    libncurses-dev \
    libssl-dev \
    libboost-atomic-dev \
    libboost-chrono-dev \
    libboost-container-dev \
    libboost-context-dev  \
    libboost-coroutine-dev  \
    libboost-date-time-dev \
    libboost-dev \
    libboost-exception-dev \
    libboost-fiber-dev \
    libboost-filesystem-dev \
    libboost-graph-dev \
    libboost-graph-parallel-dev \
    libboost-iostreams-dev \
    libboost-locale-dev \
    libboost-log-dev \
    libboost-math-dev \
    libboost-mpi-dev \
    libboost-program-options-dev \
    libboost-random-dev \
    libboost-regex-dev \
    libboost-serialization-dev \
    libboost-signals-dev \
    libboost-stacktrace-dev \
    libboost-system-dev \
    libboost-test-dev \
    libboost-thread-dev \
    libboost-timer-dev \
    libboost-tools-dev \
    libboost-type-erasure-dev \
    libboost-wave-dev && \
    apt-get autoremove && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Libraries build path
ENV BUILD_PATH /build
RUN mkdir $BUILD_PATH

# Install modern cmake (to system)
ENV CMAKE_VERSION_MAJOR 3
ENV CMAKE_VERSION_MINOR 13
ENV CMAKE_VERSION_PATCH 3
ENV CMAKE_VERSION ${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}
RUN cd $BUILD_PATH && \
    wget -qO- "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz" \
    | tar --strip-components=1 -xz -C /usr/local

################ END OF BASE ##################
# Disable "You are in 'detached HEAD' state." warning
RUN git config --global advice.detachedHead false

# # Install miniconda (python3)
# RUN mkdir -p $BUILD_PATH && \
#     wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh \
#     -O $BUILD_PATH/install_miniconda.sh && \
#     bash $BUILD_PATH/install_miniconda.sh -b -p $BUILD_PATH/miniconda && \
#     rm $BUILD_PATH/install_miniconda.sh && \
#     chmod -R +r $BUILD_PATH && \
#     chmod +x $BUILD_PATH/miniconda/bin/python
# ENV PATH $BUILD_PATH/miniconda/bin:${PATH}

# Install VTKv8 latest release
ENV VTK_GIT_TAG v8.2.0.rc2
ENV VTK_BUILD_DIR ${BUILD_PATH}/VTK-build
ENV VTK_SRC_FOLDER_NAME VTK-src
ENV VTK_SRC_DIR ${BUILD_PATH}/${VTK_SRC_FOLDER_NAME}
RUN cd $BUILD_PATH && \
    git clone https://gitlab.kitware.com/vtk/vtk.git ${VTK_SRC_FOLDER_NAME} && \
    cd $VTK_SRC_DIR && \
    git checkout ${VTK_GIT_TAG} && \
    mkdir ${VTK_BUILD_DIR} && cd ${VTK_BUILD_DIR} && \
    cmake \
        -G Ninja \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DBUILD_TESTING:BOOL=OFF \
        -DBUILD_EXAMPLES:BOOL=OFF \
        ${VTK_SRC_DIR} && \
    ninja && \
    cd ${VTK_BUILD_DIR} && \
    find . -name "*.o" -delete && \
    find ../VTK* -depth -name .git -exec rm -rf '{}' \;

# Install ITKv5 from git
ENV ITK_GIT_TAG 25b37f9f720693fd328dc38cbd7ec46d54b8501a
ENV ITK_BUILD_DIR ${BUILD_PATH}/ITK-build
ENV ITK_SRC_FOLDER_NAME ITK-src
ENV ITK_SRC_DIR ${BUILD_PATH}/${ITK_SRC_FOLDER_NAME}
RUN cd $BUILD_PATH && \
    git clone https://github.com/InsightSoftwareConsortium/ITK.git ${ITK_SRC_FOLDER_NAME} && \
    cd $ITK_SRC_DIR && \
    git checkout ${ITK_GIT_TAG} && \
    mkdir ${ITK_BUILD_DIR} && cd ${ITK_BUILD_DIR} && \
    cmake \
        -G Ninja \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DModule_ITKVtkGlue:BOOL=ON \
        -DVTK_DIR:STRING=${VTK_BUILD_DIR} \
        -DBUILD_TESTING:BOOL=OFF \
        ${ITK_SRC_DIR} && \
    ninja && \
    cd ${ITK_BUILD_DIR} && \
    find . -name "*.o" -delete && \
    find ../ITK* -depth -name .git -exec rm -rf '{}' \;

###################### DGtal #############
# Install DGtal from git
ENV DGtal_GIT_TAG 71140534477a3dd41ff8311b9176c76a52ec0b97
ENV DGtal_BUILD_DIR ${BUILD_PATH}/DGtal-build
ENV DGtal_SRC_FOLDER_NAME DGtal-src
ENV DGtal_SRC_DIR ${BUILD_PATH}/${DGtal_SRC_FOLDER_NAME}

RUN cd $BUILD_PATH && \
    git clone https://github.com/DGtal-team/DGtal.git ${DGtal_SRC_FOLDER_NAME} && \
    cd $DGtal_SRC_DIR && \
    git checkout ${DGtal_GIT_TAG} && \
    mkdir ${DGtal_BUILD_DIR} && cd ${DGtal_BUILD_DIR} && \
    cmake \
        -G Ninja \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DBUILD_SHARED_LIBS:BOOL=ON \
        -DBUILD_TESTING:BOOL=OFF \
        -DBUILD_EXAMPLES:BOOL=OFF \
        -DWITH_ITK:BOOL=ON \
        -DITK_DIR:STRING=${ITK_BUILD_DIR} \
        ${DGtal_SRC_DIR} && \
    ninja && \
    cd ${DGtal_BUILD_DIR} && \
    find . -name "*.o" -delete && \
    find ../DGtal* -depth -name .git -exec rm -rf '{}' \;

###################### SGEXT #############

ENV SGEXT_SCRIPTS_GIT_TAG master
ENV SGEXT_SCRIPTS_BUILD_DIR ${BUILD_PATH}/SGEXT_SCRIPTS-build
ENV SGEXT_SCRIPTS_SRC_FOLDER_NAME SGEXT_SCRIPTS-src
ENV SGEXT_SCRIPTS_SRC_DIR ${BUILD_PATH}/${SGEXT_SCRIPTS_SRC_FOLDER_NAME}

RUN cd $BUILD_PATH && \
    git clone https://github.com/phcerdan/SGEXT-scripts.git ${SGEXT_SCRIPTS_SRC_FOLDER_NAME} && \
    cd $SGEXT_SCRIPTS_SRC_DIR && \
    git checkout ${SGEXT_SCRIPTS_GIT_TAG} && \
    mkdir ${SGEXT_SCRIPTS_BUILD_DIR} && cd ${SGEXT_SCRIPTS_BUILD_DIR} && \
    cmake \
        -G Ninja \
        -DCMAKE_BUILD_TYPE:STRING=Release \
        -DENABLE_TESTING:BOOL=OFF \
        -DVISUALIZE:BOOL=OFF \
        -DITK_DIR:STRING=${ITK_BUILD_DIR} \
        -DDGtal_DIR:STRING=${DGtal_BUILD_DIR} \
        ${SGEXT_SCRIPTS_SRC_DIR} && \
    ninja && \
    cd ${SGEXT_SCRIPTS_BUILD_DIR} && \
    find . -name "*.o" -delete && \
    find ../SGEXT_SCRIPTS* -depth -name .git -exec rm -rf '{}' \;
ENV PATH="${SGEXT_SCRIPTS_BUILD_DIR}/cpp-scripts:${PATH}"

# Build-time metadata as defined at http://label-schema.org
ARG BUILD_DATE
ARG IMAGE=sgext/base
ARG VERSION=latest
ARG VCS_REF
ARG VCS_URL
LABEL org.label-schema.build-date=$BUILD_DATE \
      org.label-schema.name=$IMAGE \
      org.label-schema.version=$VERSION \
      org.label-schema.vcs-ref=$VCS_REF \
      org.label-schema.vcs-url=$VCS_URL \
      org.label-schema.schema-version="1.0"

