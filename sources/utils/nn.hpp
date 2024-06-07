#pragma once

#include <map>
#include <string>
#include "matrix.hpp"

namespace ActivationFunction{
	using Ptr = float (*)(float);

	ActivationFunction::Ptr Find(const std::string& name);

    ActivationFunction::Ptr FindDerivative(const std::string& name);
}

class Layer {
	Matrix<float> m_Weights;
	Matrix<float> m_Biases;
	
	ActivationFunction::Ptr m_Function;
	std::string m_FunctionName;
public:
	Layer(Layer &&layer) = default;
	Layer(const Layer &layer) = default;

	Layer &operator=(Layer &&layer) = default;
	Layer &operator=(const Layer &layer) = default;

	Layer(Matrix<float> weights, Matrix<float> biases, const std::string &function_name);

	Layer(int input_size, int output_size, ActivationFunction::Ptr function, const std::string &function_name);

	Matrix<float> Do(const Matrix<float>& input)const;

	const Matrix<float>& Weights()const;

	const Matrix<float>& Biases()const;

	Matrix<float>& Weights();

	Matrix<float>& Biases();

	const std::string& FunctionName()const;

	ActivationFunction::Ptr Function()const;

	bool IsComplete()const;

	static void Mix(Matrix<float>& output, const Matrix<float>& left, const Matrix<float>& right, float rate = 0.5f);

	static Layer Crossover(const Layer& parent1, const Layer& parent2);

	static Layer MutateLayer(const Layer& layer, float chance, float range);
};

class NeuralNetwork {
	std::vector<Layer> m_Model;
	std::vector<int> m_Topology;
	std::vector<std::string> m_Functions;
public:
	NeuralNetwork(NeuralNetwork &&) = default;
	NeuralNetwork(const NeuralNetwork &) = default;

	NeuralNetwork &operator=(NeuralNetwork &&) = default;
	NeuralNetwork &operator=(const NeuralNetwork &) = default;

	NeuralNetwork(std::vector<Layer> model, std::vector<int> topology, std::vector<std::string> functions);

	NeuralNetwork(std::vector<int> topology, std::vector<std::string> functions);

	Matrix<float> Do(Matrix<float> Input)const;

    float Backpropagation(const std::vector<std::pair<Matrix<float>, Matrix<float>>>& dataset, float learning_rate);

    Matrix<float> MeanSquaredErrorDerivative(const Matrix<float>& prediction, const Matrix<float>& target);
    
	static NeuralNetwork Crossover(const NeuralNetwork& parent1, const NeuralNetwork& parent2);

	static NeuralNetwork MutateNetwork(const NeuralNetwork& network, float chance, float range);

	const std::vector<Layer>& Layers()const;

	const std::vector<int>& Topology()const;

	const std::vector<std::string>& Functions()const;
};


template<>
struct Serializer<Layer>{
	static void ToStream(const Layer& layer, std::ostream& stream) {
		Serializer<Matrix<float>>::ToStream(layer.Weights(), stream);
		Serializer<Matrix<float>>::ToStream(layer.Biases(), stream);
		Serializer<std::string>::ToStream(layer.FunctionName(), stream);
	}

	static std::optional<Layer> FromStream(std::istream& stream) {
		auto w = Serializer<Matrix<float>>::FromStream(stream);
		auto b = Serializer<Matrix<float>>::FromStream(stream);
		auto s = Serializer<std::string>::FromStream(stream);

		if(!w.has_value() || !b.has_value() || !s.has_value())
			return std::nullopt;

		return {Layer(std::move(w.value()), std::move(b.value()), std::move(s.value()))};
	}
};

template<>
struct Serializer<NeuralNetwork> {
	static void ToStream(const NeuralNetwork& nn, std::ostream& stream) {
		Serializer<std::vector<Layer>>::ToStream(nn.Layers(), stream);
		Serializer<std::vector<int>>::ToStream(nn.Topology(), stream);
		Serializer<std::vector<std::string>>::ToStream(nn.Functions(), stream);
	}

	static std::optional<NeuralNetwork> FromStream(std::istream& stream) {
		auto l = Serializer<std::vector<Layer>>::FromStream(stream);
		auto t = Serializer<std::vector<int>>::FromStream(stream);
		auto f = Serializer<std::vector<std::string>>::FromStream(stream);
	
		if(!l.has_value() || !t.has_value() || !f.has_value())
			return std::nullopt;

		return {NeuralNetwork(std::move(l.value()), std::move(t.value()), std::move(f.value()))};
	}

};
