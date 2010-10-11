#include "vtkPTXReader.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDoubleArray.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkDenseArray.h"
#include "vtkCellArray.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkTriangleFilter.h"
#include "vtkImageReslice.h"

//for testing only
#include "vtkVertexGlyphFilter.h"

#include <sstream>

vtkStandardNewMacro(vtkPTXReader);

vtkPTXReader::vtkPTXReader()
{
  this->FileName = NULL;

  this->SetNumberOfInputPorts(0);

  this->Output = vtkSmartPointer<vtkImageData>::New();
}

void vtkPTXReader::ReadFile()
{
  std::ifstream infile;
  infile.open(this->FileName);
  if(!infile)
    {
    std::cout << "Could not open ptx file " << this->FileName << "!" << std::endl;
    return;
    }

  // Read the header
  std::string line;

  unsigned int numberOfThetaPoints; // aka numberOfColumns
  unsigned int numberOfPhiPoints; // aka numberOfRows

  getline(infile, line);
  std::stringstream(line) >> numberOfThetaPoints;

  getline(infile, line);
  std::stringstream(line) >> numberOfPhiPoints;

  // Skip 8 lines (identity matrices)
  for(int i = 0; i < 8; i++)
    {
    getline(infile, line);
    }

  std::cout << "PhiPoints: " << numberOfPhiPoints << std::endl;
  std::cout << "ThetaPoints: " << numberOfThetaPoints << std::endl;

  // Setup the grid
  vtkSmartPointer<vtkImageReslice> reslice =
    vtkSmartPointer<vtkImageReslice>::New();
  reslice->SetOutputExtent(0, numberOfThetaPoints - 1, 0, numberOfPhiPoints - 1, 0, 0);
  reslice->SetInputConnection(this->Output->GetProducerPort());
  reslice->Update();

  this->Output->ShallowCopy(reslice->GetOutput());

  int totalPoints = numberOfPhiPoints * numberOfThetaPoints;
  vtkSmartPointer<vtkDoubleArray> coordinateArray =
    vtkSmartPointer<vtkDoubleArray>::New();
  coordinateArray->SetName("CoordinateArray");
  coordinateArray->SetNumberOfComponents(3);
  coordinateArray->SetNumberOfTuples(totalPoints);

  vtkSmartPointer<vtkDoubleArray> normalArray =
    vtkSmartPointer<vtkDoubleArray>::New();
  normalArray->SetName("NormalArray");
  normalArray->SetNumberOfComponents(3);
  normalArray->SetNumberOfTuples(totalPoints);

  vtkSmartPointer<vtkIntArray> validArray =
    vtkSmartPointer<vtkIntArray>::New();
  validArray->SetName("ValidArray");
  validArray->SetNumberOfComponents(1);
  validArray->SetNumberOfTuples(totalPoints);

  vtkSmartPointer<vtkDoubleArray> intensityArray =
    vtkSmartPointer<vtkDoubleArray>::New();
  intensityArray->SetName("IntensityArray");
  intensityArray->SetNumberOfComponents(1);
  intensityArray->SetNumberOfTuples(totalPoints);


  for(unsigned int counter = 0; counter < numberOfPhiPoints*numberOfThetaPoints; counter++)
    {
    //std::cout << "counter = " << counter << std::endl;
    getline(infile, line);
    //std::cout << "line: " << line << std::endl;

    double coordinate[3];
    double intensity;
    double colorDouble[3];
    //unsigned char colorChar[3];

    std::stringstream parsedLine(line);
    parsedLine >> coordinate[0] >> coordinate[1] >> coordinate[2] >> intensity
               >> colorDouble[0] >> colorDouble[1] >> colorDouble[2];

    // Compute the grid index from the linear index
    unsigned int phi = counter / numberOfPhiPoints;
    unsigned int theta = counter % numberOfPhiPoints;

    //std::cout << "theta: " << theta << " phi: " << phi << std::endl;

    // Set the point
    coordinateArray->SetTupleValue(counter, coordinate);

    intensityArray->SetValue(counter, intensity);
    // Set the validity - the Leica HDS3000 scanner marks invalid points as "0 0 0 .5 0 0 0"
    if(coordinate[0] == 0 && coordinate[1] == 0 && coordinate[2] == 0 && intensity == 0.50
      && colorDouble[0] == 0 && colorDouble[1] == 0 && colorDouble[2] == 0)
      {
      validArray->SetValue(counter, 0);
      }
    else
      {
      validArray->SetValue(counter, 1);
      }

    unsigned char* pixel = static_cast<unsigned char*>(this->Output->GetScalarPointer(phi,theta,0));
    pixel[0] = static_cast<unsigned char>(colorDouble[0]*255.);
    pixel[1] = static_cast<unsigned char>(colorDouble[1]*255.);
    pixel[2] = static_cast<unsigned char>(colorDouble[2]*255.);

    }//end for

  this->Output->GetPointData()->AddArray(validArray);
  this->Output->GetPointData()->AddArray(coordinateArray);
  this->Output->GetPointData()->AddArray(normalArray);
  this->Output->GetPointData()->AddArray(intensityArray);

  // Close the input file
  infile.close();

}

int vtkPTXReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{

  // Get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Get the ouptut
  vtkImageData *output = vtkImageData::SafeDownCast(
          outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ReadFile();

  output->ShallowCopy(this->Output);
  output->SetWholeExtent(output->GetExtent());
  return 1;
}


void vtkPTXReader::GetValidOutputPoints(vtkPolyData* output)
{
  vtkSmartPointer<vtkPoints> points =
    vtkSmartPointer<vtkPoints>::New();

  // Declare an array to store the normals
  vtkSmartPointer<vtkDoubleArray> normals =
    vtkSmartPointer<vtkDoubleArray>::New();
  normals->SetNumberOfComponents(3);
  normals->SetName("Normals");

  // Declare an array to store the intensities
  vtkSmartPointer<vtkDoubleArray> intensities =
    vtkSmartPointer<vtkDoubleArray>::New();
  intensities->SetNumberOfComponents(1);
  intensities->SetName("Intensities");

  vtkIntArray* validArray = vtkIntArray::SafeDownCast(this->Output->GetPointData()->GetArray("ValidArray"));
  vtkDoubleArray* normalArray = vtkDoubleArray::SafeDownCast(this->Output->GetPointData()->GetArray("NormalArray"));
  vtkDoubleArray* coordinateArray = vtkDoubleArray::SafeDownCast(this->Output->GetPointData()->GetArray("CoordinateArray"));
  vtkDoubleArray* intensityArray = vtkDoubleArray::SafeDownCast(this->Output->GetPointData()->GetArray("IntensityArray"));

  int dimensions[3];
  this->Output->GetDimensions(dimensions);

  for(unsigned int index = 0; index < dimensions[0]*dimensions[1]; index++)
    {
    // If this point is not valid, skip it
    if(validArray->GetValue(index) == 0)
      {
      continue;
      }

    double coordinate[3];
    coordinateArray->GetTupleValue(index, coordinate);
    points->InsertNextPoint(coordinate);

    double normal[3];
    normalArray->GetTupleValue(index, normal);
    normals->InsertNextTupleValue(normal);

    intensities->InsertNextValue(intensityArray->GetValue(index));
  } //end for loop

  vtkSmartPointer<vtkPolyData> polydata =
    vtkSmartPointer<vtkPolyData>::New();
  polydata->SetPoints(points);
  polydata->GetPointData()->SetNormals(normals);
  polydata->GetPointData()->AddArray(intensities);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter =
    vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polydata->GetProducerPort());
  glyphFilter->Update();

  // Save these arrays in the polydata
  output->ShallowCopy(glyphFilter->GetOutput());
}

//----------------------------------------------------------------------------
void vtkPTXReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << endl;
}
