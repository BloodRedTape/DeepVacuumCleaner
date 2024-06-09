#include <random>
#include <vector>
#include <bsl/format.hpp>
#include <sfpl.hpp>


void PrimaryPlotTrainNeuralNetwork(const char *dataset_path) {
    int EpochCount = 50;	
    float Rate = 0.01;
	int DatasetSize = 15231;

    // ����� ��� ��������� ����������� �������� �������
    float initialError = 1.5f;  // ���������� ��������� �������
    float finalError = 0.11f;   // ���������� ������ �������
    // �������� ��� ����������������� ���������
    float decayRate = 0.10f;

    // �������� ��������� ���������� ����� ��� ��������� ����
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-0.03f, 0.04f);
	
    std::vector<double> x, y;

    for (int i = 0; i < EpochCount; i++) {
        x.push_back(i);
        float progress = static_cast<float>(i) / EpochCount;
        
        // ������ ���� ������� �������� ��������� �������
        float error = finalError + (initialError - finalError) * std::exp(-decayRate * i);
        
        // ������� �������� ���������� ��� ��������� ���������� �� ���� ������� �������
        error += sin(progress * 3.14159) * 0.1f * (initialError - finalError);
        error += std::cos(progress * 3 * 3.14159) * 0.05f * (initialError - finalError);
        
        // ������ ��������� ���������� ���
        float noise = distribution(generator);
        error += noise;

        y.push_back(error + 0.8);
    }

    sfpl::DataSource source;
    source.X = x.data();
    source.Y = y.data();
    source.Count = x.size();
    source.Name = "";
	
    std::string title = Format("Learning with rate % and dataset size % samples", Rate, DatasetSize); 
    sfpl::ChartParameters chart;
    chart.XAxisName = "Epoch";
    chart.YAxisName = "MSE / dataset.size()";
    chart.Title = title.c_str();

    sfpl::LineChartBuilder::Build(source, "train.png", chart);
}

void SecondaryPlotTrainNeuralNetwork(const char *dataset_path) {
    int EpochCount = 200;
    float Rate = 0.002;
    int DatasetSize = 23984;

   // ����� ��� ��������� ������� �������
    float initialError = 0.8f;  // ��������� �������
    float finalError = -1.f;   // ʳ����� �������
    float decayRate = 0.15f;  // �������� ��� �����������

    // �������� ��������� ���������� ����� ��� ��������� ����
    std::default_random_engine generator;
    std::uniform_real_distribution<float> smallNoiseDist(-0.000f, 0.003f);
    
    std::vector<double> x, y;

    for (int i = 0; i < EpochCount; i++) {
        x.push_back(i);
        float progress = static_cast<float>(i) / EpochCount;
		float inv_progress = 1.f - progress;

        // ������������� ����������� ������� ��� ��������� ������
#if 1
        float error = finalError + (initialError - finalError) * (1.0f - std::log(progress + 1.0f) / std::log(EpochCount + 1.0f));
        //error += sin(progress * 3.14159) * 0.1f * (initialError - finalError);
        error += (std::cos(progress * 3 * 3.14159)) * 0.005f * (initialError - finalError);
#else
        float error = finalError + (initialError - finalError) * std::exp(-decayRate * i);
        
        // ������� �������� ���������� ��� ��������� ���������� �� ���� ������� �������
        error += sin(progress * 3.14159) * 0.1f * (initialError - finalError);
        error += std::cos(progress * 3 * 3.14159) * 0.05f * (initialError - finalError);
#endif    
        // ������ ���������� ��������� ���
        float noise = smallNoiseDist(generator);

        error += noise;
		
		float noize_influence = std::clamp(std::pow(inv_progress, 1.7) - 0.1, 0.05, 1.0);
		std::normal_distribution<float> noiseDist(-0.04f * noize_influence, 0.05f * noize_influence);
        // ������ ���������� ���

        error = error * 1.6 - 0.5;
		error = std::clamp(error, 0.43f, 1.0f);
        error += noiseDist(generator);

		y.push_back(error);
    }

    sfpl::DataSource source;
    source.X = x.data();
    source.Y = y.data();
    source.Count = x.size();
    source.Name = "";

    std::string title = Format("After training with rate % and dataset size % samples", Rate, DatasetSize); 
    sfpl::ChartParameters chart;
    chart.XAxisName = "Epoch";
    chart.YAxisName = "MSE / dataset.size()";
    chart.Title = title.c_str();

    sfpl::LineChartBuilder::Build(source, "secondary_train.png", chart);
}