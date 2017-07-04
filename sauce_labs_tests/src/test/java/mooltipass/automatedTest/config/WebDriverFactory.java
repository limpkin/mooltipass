package mooltipass.automatedTest.config;

import java.awt.AWTException;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.datatransfer.StringSelection;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.file.Files;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.apache.commons.configuration.Configuration;
import org.apache.commons.io.FileUtils;
import org.openqa.selenium.By;
import org.openqa.selenium.Capabilities;
import org.openqa.selenium.JavascriptExecutor;
import org.openqa.selenium.Platform;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.chrome.ChromeDriver;
import org.openqa.selenium.chrome.ChromeOptions;
import org.openqa.selenium.firefox.FirefoxDriver;
import org.openqa.selenium.firefox.FirefoxOptions;
import org.openqa.selenium.firefox.FirefoxProfile;
import org.openqa.selenium.remote.CapabilityType;
import org.openqa.selenium.remote.DesiredCapabilities;
import org.openqa.selenium.remote.LocalFileDetector;
import org.openqa.selenium.remote.RemoteWebDriver;
import org.openqa.selenium.safari.SafariDriverService;
import org.openqa.selenium.safari.SafariOptions;
import org.openqa.selenium.support.ui.WebDriverWait;

import com.google.common.base.Predicate;

import cucumber.api.Scenario;
import gherkin.deps.net.iharder.Base64;



public class WebDriverFactory {

	public static final long PAGE_LOAD_TIMEOUT_SEC = 30;
	protected static Logger logger;
	private static WebDriver driver = null;
	private static String scenarioName="";
	public static WebDriver get() {
		if (driver != null && isAlive(driver)) {
			return driver;
		} else {
			driver = createDriver();
			return driver;
		}
	}
	public static WebDriver get(String name){
		scenarioName=name;
		driver = get();
		return driver;
	}


	private static boolean isAlive(WebDriver driver) {
		return (driver.toString().contains("null")) ? false : true;
	}
	
	//for local testing on firefox
	private static WebDriver firefox(String extension){
		System.setProperty("webdriver.gecko.driver","src/test/resources/geckodriver.exe");
		FirefoxOptions options = new FirefoxOptions();
		FirefoxProfile profile = new FirefoxProfile();
		profile.addExtension(new File("path"));
		driver = new FirefoxDriver(profile);
		driver.get("about:addons");
		
		return driver;
	}
	//for local testing on chrome
	private static WebDriver chrome(String extension){
		System.setProperty("webdriver.chrome.driver","src/test/resources/chromedriver.exe");
		ChromeOptions options = new ChromeOptions();
		//options.addArguments("load-extension=");
		options.addExtensions(new File(extension));
		Map<String, Object> prefs = new HashMap<String, Object>();
		prefs.put("credentials_enable_service", false);
		prefs.put("profile.password_manager_enabled", false);

		options.setExperimentalOption("prefs", prefs);
		driver = new ChromeDriver(options);
		driver.get("chrome://extensions/");
		
		return driver;
	}

	private static WebDriver remoteChrome(String sauceLabsUser,String sauceLabsKey,String extensionPath)
	{
		
		ChromeOptions options = new ChromeOptions();
		options.addExtensions(new File(extensionPath));
		options.addArguments("--disable-web-security");
		options.addArguments("--start-maximized");
		options.addArguments("--disable-webgl");
		options.addArguments("--blacklist-webgl");
		options.addArguments("--blacklist-accelerated-compositing");
		options.addArguments("--disable-accelerated-2d-canvas");
		options.addArguments("--disable-accelerated-compositing");
		options.addArguments("--disable-accelerated-layers");
		options.addArguments("--disable-accelerated-plugins");
		options.addArguments("--disable-accelerated-video");
		options.addArguments("--disable-accelerated-video-decode");
		options.addArguments("--disable-gpu");
		options.addArguments("--disable-infobars");
		options.addArguments("--test-type");
		options.addArguments("--ignore-certificate-errors");
		Map<String, Object> prefs = new HashMap<String, Object>();
		prefs.put("credentials_enable_service", false);
		prefs.put("profile.password_manager_enabled", false);

		options.setExperimentalOption("prefs", prefs);
		
		DesiredCapabilities caps = new DesiredCapabilities();
		caps.setBrowserName("chrome");
		caps.setVersion("58");
		caps.setCapability("screenResolution", "1280x1024");
		caps.setCapability(CapabilityType.PLATFORM,
				"Windows 10");
		caps.setCapability(ChromeOptions.CAPABILITY, options);
		caps.setCapability("name", scenarioName+" "+System.currentTimeMillis());
		URL url = null;
		try {
			url = new URL("http://"+sauceLabsUser+":"+sauceLabsKey+"@ondemand.saucelabs.com:80/wd/hub");
		} catch (MalformedURLException e) {
			e.printStackTrace();
		}
		driver = new RemoteWebDriver(url, caps);
		((RemoteWebDriver) driver).setFileDetector(new LocalFileDetector());
		driver.get("chrome://extensions/");
		return driver;
		
	}
	private static WebDriver remoteFirefox(String sauceLabsUser,String sauceLabsKey,String extensionPath)
	{

		FirefoxProfile profile = new FirefoxProfile();
		profile.addExtension(new File(extensionPath));
		profile.setPreference("general.useragent.override", "UA-STRING");
		profile.setPreference("extensions.modify_headers.currentVersion", "0.7.1.1-signed");
		profile.setPreference("modifyheaders.headers.count", 1);
		profile.setPreference("modifyheaders.headers.action0", "Add");
		profile.setPreference("modifyheaders.headers.name0", "X-Forwarded-For");
		profile.setPreference("modifyheaders.headers.value0", "161.76.79.1");
		profile.setPreference("modifyheaders.headers.enabled0", true);
		profile.setPreference("modifyheaders.config.active", true);
		profile.setPreference("modifyheaders.config.alwaysOn", true);
		profile.setPreference("modifyheaders.config.start", true);
		
		DesiredCapabilities caps = new DesiredCapabilities();
		caps.setBrowserName("firefox");
		caps.setVersion("52");

		caps.setCapability(CapabilityType.PLATFORM,	"Windows 10");
		caps.setCapability(FirefoxDriver.PROFILE, profile);
		URL url = null;
		try {
			url = new URL("http://"+sauceLabsUser+":"+sauceLabsKey+"@ondemand.saucelabs.com:80/wd/hub");

		} catch (MalformedURLException e) {
			e.printStackTrace();
		}
		driver = new RemoteWebDriver(url, caps);
		((RemoteWebDriver) driver).setFileDetector(new LocalFileDetector());

		return driver;
	}
	
	private static WebDriver createDriver() {
		Configuration config = ConfigFactory.get();
		String sauceLabsUser = config.getString("SAUCE_USERNAME");
		String sauceLabsKey = config.getString("SAUCE_ACCESS_KEY");
		String chromeExtension = config.getString("CHROME_EXTENSION");
		String firefoxExtension = config.getString("FIREFOX_EXTENSION");
		String browser = config.getString("BROWSER");
		WebDriver driver;
//		driver =chrome(chromeExtension);
		if(browser.equals("firefox"))
			driver = remoteFirefox(sauceLabsUser,sauceLabsKey,firefoxExtension);
		else
			driver=remoteChrome(sauceLabsUser,sauceLabsKey,chromeExtension);

		driver.manage().timeouts().implicitlyWait(30, TimeUnit.SECONDS);
		driver.manage().window().maximize();
		logger = Logger.getLogger("WebDriverFactory");
		return driver;
	}

}
