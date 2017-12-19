package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.JavascriptExecutor;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebDriverException;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class PcbWay extends AbstractPage{
	
	public PcbWay (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//span[@class='account-unsigned']//a[contains(text(),'Sign in')]")
	private WebElement loginBtn;
	@FindBy(id = "email2")
	private WebElement email;
	
	@FindBy(id = "pwd3")
	private WebElement password;
	
	@FindBy(xpath = "//input[@class='b-buton login-button']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[contains(text(),'Sign Out')]")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//div[@class='nav-ubox']")
	private WebElement dashBoard;
	
	@FindBy(xpath = "//div[@class='olark-top-bar-button' and @aria-hidden='true']")
	private WebElement popup;
	
	
	public void goToLogin(){	
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}

	
	public void submit(){
		try {
		submitLogin.click();
		}catch(WebDriverException e) {
			popup.click();
			submitLogin.click();
		}
	}

	public void logout(){	

		waitUntilAppears(logoutBtn);
		logoutBtn.click();
	}
	public void goTodDashboard()
	{
		JavascriptExecutor executor = (JavascriptExecutor)driver;
		executor.executeScript("$('.nav-ubox').addClass('nav-ubox-active')");
		//waitUntilAppears(dashBoard);
		//hover(dashBoard);

	}
	public boolean checkLogin(){
		waitUntilAppears(By.id("login-stat-signed"));
		return isElementPresent(By.id("login-stat-signed"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("email2"));
	}


}
