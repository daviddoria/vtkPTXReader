cmake_minimum_required(VERSION 2.6)

PROJECT(vtkPTXReader)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

ADD_EXECUTABLE(vtkPTXReaderExample vtkPTXReaderExample.cxx vtkPTXReader.cxx)
TARGET_LINK_LIBRARIES(vtkPTXReaderExample vtkHybrid )

