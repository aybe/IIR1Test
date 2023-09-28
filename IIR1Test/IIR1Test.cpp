// ReSharper disable StringLiteralTypo
// ReSharper disable CppInconsistentNaming
// ReSharper disable IdentifierTypo
#include"Iir.h"
#include <sndfile.h>
#include <algorithm>

int main()
{
	const auto sourcePath = "input.wav";
	const auto targetPath = "iir.wav";

	SF_INFO info = {};

	const auto sourceFile = sf_open(sourcePath, SFM_READ, &info);

	assert(sourceFile);

	const int channels = info.channels;

	const auto targetFile = sf_open(targetPath, SFM_WRITE, &info);

	assert(targetFile);

	constexpr int order = 32;

	auto filters = std::vector<Iir::Butterworth::LowPass<order>>(channels);

	for (int i = 0; i < channels; ++i)
	{
		filters[i].setup(44100.0, 11025.0);
	}

	constexpr size_t bufferSize = 16384;

	std::vector<double> sourceBuffer(bufferSize * channels);
	std::vector<double> targetBuffer(sourceBuffer.size());

	sf_count_t read;

	do
	{
		read = sf_readf_double(sourceFile, sourceBuffer.data(), bufferSize);

		for (int i = 0; i < channels; ++i)
		{
			auto filter = filters[i];

			for (int j = 0; j < read; ++j)
			{
				const auto k = j * channels + i;

				const auto sample = sourceBuffer[k];

				const auto result = filter.filter(sample);

				targetBuffer[k] = std::clamp(result, -1.0, +1.0);
			}
		}

		const auto count = sf_writef_double(targetFile, targetBuffer.data(), read);

		assert(count == read);
	}
	while (read == bufferSize);

	const int close1 = sf_close(sourceFile);

	assert(close1 == 0);

	const int close2 = sf_close(targetFile);

	assert(close2 == 0);
}
