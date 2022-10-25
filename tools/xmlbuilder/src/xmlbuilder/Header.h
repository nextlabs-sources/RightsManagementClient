#pragma once
#include <string>
#define MAXPAGESIZE	4

const std::wstring welcomeText = L"This packaging tool can be used to create a Rights Management Client installation package.\n\nAfter the installation package has been created, you can deploy it to multiple machines on your network.\n\nThe Rights Management Client Packager Tool creates two versions of the installer (.exe and .msi).";

const std::wstring bannerTitle[MAXPAGESIZE] = {
	L"",
	L"Platform and Language",
	L"Web Service URL and Destination",
	L"Ready to Build the Package"
};

const std::wstring bannerText[MAXPAGESIZE] = {
	L"",
	L"Select the platform(s) and language on which you want to deploy the Rights Management Client installation package.",
	L"Type the URL of the Policy Server web service and the destination of the package.",
	L"Review the installation details before you click Finish."
};
