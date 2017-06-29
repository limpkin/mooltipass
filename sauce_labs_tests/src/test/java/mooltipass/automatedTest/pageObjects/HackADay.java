package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class HackADay extends AbstractPage{
	
	public HackADay (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//a[contains(text(),'Log In')]")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//input[@name='email']")
	private WebElement email;

	@FindBy(xpath = "//input[@name='password']")
	private WebElement password;
	
	@FindBy(xpath = "//button[@class='gold-gold-button button-signin']")
	private WebElement submitLogin;
	
	
	@FindBy(xpath = "//a[contains(text(),'Logout')]")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//img[@class='user-image']")
	private WebElement dashBoard;
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void submit(){
		submitLogin.click();
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		hover(dashBoard);
		
	}
	public void logout(){
		waitUntilAppears(logoutBtn);
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath( "//img[@class='user-image']"));
		return isElementPresent(By.xpath( "//img[@class='user-image']"));
	}
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.xpath("//input[@name='email']"));
		return isElementPresent(By.xpath("//input[@name='email']"));
	}

}
