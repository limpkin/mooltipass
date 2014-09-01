/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/* \file    AftBaes.java
 * \brief   Automatic Functional Test Base using Selenium
 *  Created: 12/08/2014 20:46:42
 *  Author: Erik G. H. Meade
 */
package mooltipass;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;

import org.apache.commons.io.FileUtils;
import org.junit.BeforeClass;
import org.openqa.selenium.OutputType;
import org.openqa.selenium.TakesScreenshot;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.chrome.ChromeDriver;
import org.openqa.selenium.chrome.ChromeDriverService;
import org.openqa.selenium.chrome.ChromeOptions;
import org.openqa.selenium.remote.DesiredCapabilities;

/**
 * 0. MP requires using the Chrome Developer Channel version of Chrome.
 * 1. Download and unzip chromedriver - http://chromedriver.storage.googleapis.com/index.html
 * 2. When running this test set -Dwebdriver.chrome.driver=path_to_chromedriver
 * 3. When you see the message "Waiting 2 minutes for you to install Extensions manually." install chrome.hid-app and chrome.ext manually (Click the Developement
 * Mode checkbox in the upper right of chromes extensions page, then click Load Unpacked Extension..., and select from mooltipass/authentication_clients/chrome.hid-app then chrome.ext 
 * 
 * Base class for Automated Functional Tests, don't try to run this class as a test run one of its subclasses.
 * 
 * TODO:
 * Figure out how not to need to install extensions manually - at time of writing google only allows the loading of one unpackage extension via load-extension
 * 
 * @author eghm
 * 
 */
public class AftBase
{
	static WebDriver driver;
	static ChromeDriverService chromeDriverService;
	public static final String CHROME = "webdriver.chrome.driver";
	public static final String MOOLTIPASS_DIR = "mooltipass.dir";
	public static final String MOOLTIPASS_EXTENSION_TIMEOUT = "mooltipass.extension.timeout.seconds";
	public static final int MOOLTIPASS_EXTENSION_TIMEOUT_DEFAULT = 120;
	public static final String MOOLTIPASS_TIMEOUT = "mooltipass.timeout.seconds";
	public static final int MOOLTIPASS_TIMEOUT_DEFALT = 30;

	@BeforeClass
	public static void beforeClass() throws Exception
	{
		chromeDriverService = chromeDriverService();
		if (chromeDriverService != null)
		{
			chromeDriverService.start();
		}	
		
		ChromeOptions options = new ChromeOptions();

// Appears you can only load one unpacked extensions this way...
// list doesn't do it
//		List<String> arguments = new LinkedList<String>();
//		arguments.add("load-extension=" + System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome.hid-app");
//		arguments.add("load-extension=" + System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome-ext");
//		options.addArguments(arguments);
// comma delimited as some chrome arguments use doesn't work
//		options.addArguments("load-extension=" + System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome.hid-app","load-extension=" + System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome-ext");
// String... doesn't work		
//		options.addArguments("load-extension=" + System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome.hid-app," + System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome-ext");

// Requires extensions are crx (zip file with public and private keys)
//		List extensions = new LinkedList();
//		extensions.add(new File(System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome.hid-app"));
//		extensions.add(new File(System.getProperty(MOOLTIPASS_DIR) + File.separator + "authentication_clients" + File.separator + "chrome-ext"));
//		options.addExtensions(extensions);

		DesiredCapabilities capabilities = DesiredCapabilities.chrome();
		capabilities.setCapability(ChromeOptions.CAPABILITY, options);

		System.out.println("Starting an instance of chrome.");
		driver = new ChromeDriver(capabilities);

		// TODO figure out how to pre-install extensions automatically (see commented out code above for things that don't work)
		driver.get("chrome://extensions/");
		System.out.println("\nWaiting " + getTimeoutExtension()/1000 + " seconds for you to install Extensions manually."); 
		System.out.println("Check the Developer mode checkbox and then use the Load unpackaged extensions to load the mooltipass authentication_clients chrome.hid-app and chrome.ext\n");
		try
		{
			Thread.sleep(getTimeoutExtension());
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}	
	}

	public static ChromeDriverService chromeDriverService()
	{
		if (System.getProperty(CHROME) == null)
		{
			System.out.println("-D" + CHROME + " must be set to chromedriver path see http://chromedriver.storage.googleapis.com/index.html");
			System.exit(1);
		}

		try
		{
			ChromeDriverService chromeDriverService = new ChromeDriverService.Builder()
					.usingDriverExecutable(new File(System.getProperty(CHROME)))
					.usingAnyFreePort().build();
			return chromeDriverService;
		}
		catch (Throwable t)
		{
			throw new RuntimeException("Exception starting chrome driver service, is chromedriver ( http://chromedriver.storage.googleapis.com/index.html ) installed? You can include the path to it using -D" + CHROME, t);
		}
	}

	public int getTimeout() {
		return Integer.parseInt(System.getProperty(MOOLTIPASS_TIMEOUT, MOOLTIPASS_TIMEOUT_DEFALT + "")); 
	}
	
	public static int getTimeoutExtension() {
		return Integer.parseInt(System.getProperty(MOOLTIPASS_EXTENSION_TIMEOUT, MOOLTIPASS_EXTENSION_TIMEOUT_DEFAULT + "")) * 1000; 
	}
	
	public void screenshot() {
		if (driver instanceof TakesScreenshot) {
			File fileOutputType = ((TakesScreenshot)driver).getScreenshotAs(OutputType.FILE);
			String currentUrl = driver.getCurrentUrl();
			String baseUrl = currentUrl.substring(currentUrl.indexOf("://") + 3, currentUrl.length());
			baseUrl = baseUrl.substring(0, baseUrl.indexOf("/"));
			
			try {
				FileUtils.copyFile(fileOutputType, new File(baseUrl + "-" + Calendar.getInstance().getTime().getTime() + ".png"));
			} catch (IOException e) {
				System.err.println("Exception writing screenshot for " + baseUrl + " " + e.getMessage());
				e.printStackTrace();
			}
		}
		else
		{
			System.err.println("Screenshots not available WebDriver " + driver.getClass().toString() +
					" is not of type TakesScreenshot.");			
		}
	}
}
