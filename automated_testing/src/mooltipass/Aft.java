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
/* \file    Aft.java
 * \brief   Automatic Functional Test using Selenium and BETATESTERS_AUTOACCEPT_SETUP hex
 *  Created: 10/08/2014 13:29:42
 *  Author: Erik G. H. Meade
 */
package mooltipass;

import java.io.File;
import java.util.LinkedList;
import java.util.List;

import org.junit.BeforeClass;
import org.junit.Test;
import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.chrome.ChromeDriver;
import org.openqa.selenium.chrome.ChromeDriverService;
import org.openqa.selenium.chrome.ChromeOptions;
import org.openqa.selenium.remote.DesiredCapabilities;
import org.openqa.selenium.support.ui.ExpectedConditions;
import org.openqa.selenium.support.ui.WebDriverWait;

/**
 * 1. Download and install chromedriver - http://chromedriver.storage.googleapis.com/index.html
 * 2. When running this test set -Dwebdriver.chrome.driver=/path/to/chromedriver
 * 3. When you see the message "Waiting 2 minutes for you to install Extensions manually." install chrome.hid-app and chrome.ext manually
 * 
 * TODO:
 * Figure out how not to need to install extensions manually
 * Put test url and success criteria in external file.
 * 
 * @author eghm
 * 
 */
public class Aft
{
	static WebDriver driver;
	static ChromeDriverService chromeDriverService;
	public static final String CHROME = "webdriver.chrome.driver";
	public static final String[][] DATA = new String[][]{{"http://www.moddb.com/", "sign in", "logout"}};
	public static final String MOOLTIPASS_DIR = "mooltipass.dir";
	public static final int WAIT_TIMEOUT_SECONDS = 30;

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
		driver = new ChromeDriver(capabilities);

		// TODO figure out how to pre-install extensions automatically (see commented out code above for things that don't work)
		driver.get("chrome://extensions/");
		System.out.println("Waiting 2 minutes for you to install Extensions (chrome.hid-app and chrome.ext) manually.");
		System.out.println("Check the Developer mode checkbox and then use the Load unpackaged extensions to load the mooltipass authentication_clients chrome.hid-app and chrome.ext");
		try
		{
			Thread.sleep(120000);
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

	// TODO define urls and success criteria in file (logout link)
	@Test
	public void testAutoLoginSlashdot() throws Exception
	{
		testAutoLogin("http://slashdot.org/?nobeta=1", "Login", "Log out");
	}

	@Test
	public void testAutoLoginArtima() throws Exception
	{
		testAutoLogin(new String[]{"http://www.artima.com/sign_in?d=%2Findex.jsp", "Forgot your password?", "Sign Out"});
	}

	// https://members.cafepress.com/login.aspx
	@Test
	public void testAutoLoginCafepress() throws Exception
	{
		testAutoLogin("https://members.cafepress.com/login.aspx,SIGN IN,SIGN OUT");
	}
	
	@Test
	public void testAutoLoginData() throws Exception
	{
		testAutoLogin(DATA);
	}
	

	void testAutoLogin(String data) throws Exception
	{
		testAutoLogin(data.split(","));
	}
	
	void testAutoLogin(String[] data) throws Exception
	{
		testAutoLogin(data[0], data[1], data[2]);
	}

	void testAutoLogin(String[][] data) throws Exception
	{
		for (String[] tests : data) {
			testAutoLogin(tests);
		}
	}

	void testAutoLogin(String loginUrl, String loginLoadedLinkText, String logoutLinkText) throws Exception
	{
		driver.get(loginUrl);
		WebDriverWait wait = new WebDriverWait(driver, WAIT_TIMEOUT_SECONDS);
		// wait for Login, but we are not gonna click it testing AUTO_ACCEPT works
		wait.until(ExpectedConditions.elementToBeClickable(By .linkText(loginLoadedLinkText)));
		WebElement logout = wait.until(ExpectedConditions.elementToBeClickable(By.linkText(logoutLinkText)));
		// probably will be some cases where MP AUTO_ACCEPTS back in after we logout
		logout.click();
	}
}
