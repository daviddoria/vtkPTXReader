#include "vtkPTXReader.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageData.h>

int main (int argc, char *argv[])
{
  if(argc != 3)
    {
    std::cout << "Required arguments: InputFilename(ptx) OutputFilename(vtp)" << std::endl;
    return EXIT_FAILURE;
    }

  std::string inputFilename = argv[1];
  std::string outputFilename = argv[2];

  vtkSmartPointer<vtkPTXReader> reader =
    vtkSmartPointer<vtkPTXReader>::New();
  reader->SetFileName(inputFilename.c_str());
  reader->Update();

  vtkSmartPointer<vtkPolyData> polydata =
    vtkSmartPointer<vtkPolyData>::New();
  reader->GetValidOutputPoints(polydata);

  vtkSmartPointer<vtkXMLPolyDataWriter> polydataWriter =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  polydataWriter->SetFileName("polydata.vtp");
  polydataWriter->SetInputConnection(polydata->GetProducerPort());
  polydataWriter->Write();

  vtkSmartPointer<vtkXMLImageDataWriter> writer =
    vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(outputFilename.c_str());
  writer->SetInputConnection(reader->GetOutputPort());
  writer->Write();

  return EXIT_SUCCESS;
}
